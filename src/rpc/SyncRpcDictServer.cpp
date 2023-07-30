/*
 * Licensed under the MIT License <http://opensource.org/licenses/MIT>.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022-2023 https://github.com/klappdev
 *
 * Permission is hereby  granted, free of charge, to any  person obtaining a copy
 * of this software and associated  documentation files (the "Software"), to deal
 * in the Software  without restriction, including without  limitation the rights
 * to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
 * copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
 * IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
 * FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
 * AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
 * LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "rpc/SyncRpcDictServer.hpp"
#include "logging/Logging.hpp"
#include "common/DictCommand.hpp"

#include <thread>
#include <charconv>

static constexpr const char* const TAG = "SyncRpcpDictServer";

using namespace std::string_literals;
using namespace std::chrono_literals;

namespace lynx {

	SyncRpcDictServer::SyncRpcDictServer(const std::string& host, uint16_t port)
		: mHost(host)
		, mPort(port)
		, mService(nullptr)
		, mDictDao(host)
		, mStarted(false) {
		log::info(TAG, "Create server");
	}

	SyncRpcDictServer::~SyncRpcDictServer() {
		mStarted = false;
		log::info(TAG, "Destroy server");
	}

	bool SyncRpcDictServer::isStarted() const { return mStarted; }

	auto SyncRpcDictServer::getDictDao() const -> const SyncDictDao& { return mDictDao; }

	void SyncRpcDictServer::start() {
		log::info(TAG, "Start server");

		const std::string serverAddress = mHost + ":" + std::to_string(mPort);

		grpc::ServerBuilder builder;
		builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
		builder.RegisterService(this);

		mService = builder.BuildAndStart();
		log::info(TAG, "Start server on: %s", serverAddress.c_str());

		mStarted = true;
		mDictDao.start();

		mService->Wait();
	}

	void SyncRpcDictServer::stop() {
		mStarted = false;
		mDictDao.stop();

		std::thread shutdownThread([](grpc::Server& server) {
			log::debug(TAG, "Start shudown rpc server");

			auto deadline = std::chrono::system_clock::now() + 3s;
			server.Shutdown(deadline);

			log::debug(TAG, "Rpc server is shutdown");
		}, std::ref(*mService));
		shutdownThread.join();

		log::info(TAG, "Stop server");
	}

	auto SyncRpcDictServer::Quit(grpc::ServerContext* context, const google::protobuf::Empty* request,
								 google::protobuf::Empty* response) -> grpc::Status  {
		log::debug(TAG, "Process %s response", QUIT_COMMAND);
		stop();

		return grpc::Status::OK;
	}

	auto SyncRpcDictServer::InsertWord(grpc::ServerContext* context, const pb::RemoteWord* request,
									   google::protobuf::Empty* response) -> grpc::Status {
		BOOST_ASSERT(context);
		BOOST_ASSERT(request);
		BOOST_ASSERT(response);
		log::debug(TAG, "Process %s response", INSERT_COMMAND);

		const Word remoteWord = mParser.convert(*request);

		boost::system::result<void> operationStatus = mDictDao.insert(remoteWord);

		if (operationStatus.has_error()) {
			log::error(TAG, "Db insert word error: %s", operationStatus.error().message().c_str());
			return grpc::Status(grpc::StatusCode::INTERNAL, "Db insert word error", operationStatus.error().message().c_str());
		}

		log::debug(TAG, "Db insert word success");

		return grpc::Status::OK;
	}

	auto SyncRpcDictServer::UpdateWord(grpc::ServerContext* context, const pb::RemoteWord* request,
									   google::protobuf::Empty* response) -> grpc::Status {
		BOOST_ASSERT(context);
		BOOST_ASSERT(request);
		BOOST_ASSERT(response);
		log::debug(TAG, "Process %s response", UPDATE_COMMAND);

		const Word remoteWord = mParser.convert(*request);

		boost::system::result<void> operationStatus = mDictDao.update(remoteWord);

		if (operationStatus.has_error()) {
			log::error(TAG, "Db update word error: %s", operationStatus.error().message().c_str());
			return grpc::Status(grpc::StatusCode::INTERNAL, "Db update word error", operationStatus.error().message().c_str());
		}

		log::debug(TAG, "Db update word success");

		return grpc::Status::OK;
	}

	auto SyncRpcDictServer::DeleteWord(grpc::ServerContext* context, const rpc::WordIdRequest* request,
									   google::protobuf::Empty* response) -> grpc::Status  {
		BOOST_ASSERT(context);
		BOOST_ASSERT(request);
		BOOST_ASSERT(response);
		log::debug(TAG, "Process %s response", DELETE_COMMAND);

		const uint64_t wordId = request->id();

		boost::system::result<void> operationStatus = mDictDao.remove(wordId);

		if (operationStatus.has_error()) {
			log::error(TAG, "Db delete word id=%lu error: %s", wordId, operationStatus.error().message().c_str());
			return grpc::Status(grpc::StatusCode::INTERNAL, "Db delete word error", operationStatus.error().message().c_str());
		}

		log::debug(TAG, "Db delete word id=%lu success", wordId);

		return grpc::Status::OK;
	}

	auto SyncRpcDictServer::GetByIdWord(grpc::ServerContext* context, const rpc::WordIdRequest* request,
										pb::RemoteWord* response) -> grpc::Status  {
		BOOST_ASSERT(context);
		BOOST_ASSERT(request);
		BOOST_ASSERT(response);
		log::debug(TAG, "Process %s response", GET_BY_ID_COMMAND);

		const uint64_t wordId = request->id();

		boost::system::result<Word> localWord = mDictDao.getById(wordId);

		if (localWord.has_error()) {
			log::error(TAG, "Db get word by id=%lu error: %s", wordId, localWord.error().message().c_str());
			return grpc::Status(grpc::StatusCode::INTERNAL, "Db get word by id error", localWord.error().message().c_str());
		}

		*response = mParser.convert(localWord.value());

		log::debug(TAG, "Db get word by id=%lu success", wordId);

		return grpc::Status::OK;
	}

	auto SyncRpcDictServer::GetAllWords(grpc::ServerContext* context, const google::protobuf::Empty* request,
										rpc::ListWordsResponse* response) -> grpc::Status  {
		BOOST_ASSERT(context);
		BOOST_ASSERT(request);
		BOOST_ASSERT(response);
		log::debug(TAG, "Process %s response", GET_ALL_COMMAND);

		boost::system::result<std::vector<Word>> localWords = mDictDao.getAll();

		if (localWords.has_error()) {
			log::error(TAG, "Db get all words error: %s", localWords.error().message().c_str());
			return grpc::Status(grpc::StatusCode::INTERNAL, "Db get all words error", localWords.error().message().c_str());
		}

		for (const Word& localWord : localWords.value()) {
			pb::RemoteWord* remoteWord = response->add_words();
			BOOST_ASSERT(remoteWord);
			*remoteWord = mParser.convert(localWord);
		}

		log::debug(TAG, "Db get all words success");

		return grpc::Status::OK;
	}
}
