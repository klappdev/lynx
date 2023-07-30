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

#include "rpc/SyncRpcDictClient.hpp"
#include "logging/Logging.hpp"

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>

#include <thread>

static constexpr const char* const TAG = "SyncRpcDictClient";

using namespace std::string_literals;
using namespace std::chrono_literals;

namespace lynx {

	SyncRpcDictClient::SyncRpcDictClient(const std::string& host, uint16_t port)
		: mHost(host)
		, mPort(port)
		, mService(nullptr)
		, mStarted(false) {
		log::info(TAG, "Create client");
	}

	SyncRpcDictClient::~SyncRpcDictClient() {
		mStarted = false;
		log::info(TAG, "Destroy client");
	}

	bool SyncRpcDictClient::isStarted() const { return mStarted; }

	void SyncRpcDictClient::start() {
		log::info(TAG, "Start client");

		const std::string clientAddress = mHost + ":" + std::to_string(mPort);
		std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(clientAddress, grpc::InsecureChannelCredentials());

		mService = rpc::RemoteDictService::NewStub(std::static_pointer_cast<grpc::ChannelInterface>(channel));
		mStarted = true;
	}

	void SyncRpcDictClient::stop() {
		mStarted = false;
		log::info(TAG, "Stop client");
	}

	void SyncRpcDictClient::performQuit() {
		grpc::ClientContext context;
		google::protobuf::Empty request;
		google::protobuf::Empty response;

		const grpc::Status status = mService->Quit(&context, request, &response);

		if (status.ok()) {
			log::debug(TAG, "Perform quit service success");
		} else {
			log::error(TAG, "Can't quit service error: %d, %s, %s", status.error_code(),
					   status.error_message().c_str(), status.error_details().c_str());
		}

		std::this_thread::sleep_for(1s);
	}

	void SyncRpcDictClient::performInsert(const Word& word) {
		grpc::ClientContext context;
		google::protobuf::Empty response;

		const pb::RemoteWord remoteWord = mParser.convert(word);

		const grpc::Status status = mService->InsertWord(&context, remoteWord, &response);

		if (status.ok()) {
			log::debug(TAG, "Perform remote insert word success");
		} else {
			log::error(TAG, "Can't insert word error: %d, %s, %s", status.error_code(),
					   status.error_message().c_str(), status.error_details().c_str());
		}
	}

	void SyncRpcDictClient::performUpdate(const Word& word) {
		grpc::ClientContext context;
		google::protobuf::Empty response;

		const pb::RemoteWord remoteWord = mParser.convert(word);

		const grpc::Status status = mService->UpdateWord(&context, remoteWord, &response);

		if (status.ok()) {
			log::debug(TAG, "Perform remote update word success");
		} else {
			log::error(TAG, "Can't update word error: %d, %s, %s", status.error_code(),
					   status.error_message().c_str(), status.error_details().c_str());
		}
	}

	void SyncRpcDictClient::performDelete(uint64_t id) {
		grpc::ClientContext context;
		google::protobuf::Empty response;

		rpc::WordIdRequest remoteWordId;
		remoteWordId.set_id(id);

		const grpc::Status status = mService->DeleteWord(&context, remoteWordId, &response);

		if (status.ok()) {
			log::debug(TAG, "Perform remote delete word success");
		} else {
			log::error(TAG, "Can't delete word error: %d, %s, %s", status.error_code(),
					   status.error_message().c_str(), status.error_details().c_str());
		}
	}

	auto SyncRpcDictClient::performGetById(uint64_t id) -> Word {
		grpc::ClientContext context;
		pb::RemoteWord remoteWord;

		rpc::WordIdRequest remoteWordId;
		remoteWordId.set_id(id);

		const grpc::Status status = mService->GetByIdWord(&context, remoteWordId, &remoteWord);

		if (status.ok()) {
			log::debug(TAG, "Perform remote get by id word success");
		} else {
			log::error(TAG, "Can't get word by id error: %d, %s, %s", status.error_code(),
					   status.error_message().c_str(), status.error_details().c_str());
			return {};
		}

		return mParser.convert(remoteWord);
	}

	auto SyncRpcDictClient::performGetAll() -> std::vector<Word> {
		grpc::ClientContext context;
		google::protobuf::Empty request;
		rpc::ListWordsResponse remoteWords;
		std::vector<Word> localtWords;

		const grpc::Status status = mService->GetAllWords(&context, request, &remoteWords);

		if (status.ok()) {
			log::debug(TAG, "Perform remote get all words success");
		} else {
			log::error(TAG, "Can't get all words error: %d, %s, %s", status.error_code(),
					   status.error_message().c_str(), status.error_details().c_str());
			return {};
		}

		for (int32_t i = 0; i < remoteWords.words_size(); ++i) {
			const pb::RemoteWord& remoteWord = remoteWords.words(i);
			localtWords.push_back(mParser.convert(remoteWord));
		}

		return localtWords;
	}
}
