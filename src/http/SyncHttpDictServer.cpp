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

#include "http/SyncHttpDictServer.hpp"
#include "logging/Logging.hpp"
#include "util/StringUtils.hpp"

#include <thread>
#include <charconv>

#include <boost/beast/version.hpp>

static constexpr const char* const TAG = "SyncHttpDictServer";
static constexpr const char* const SERVER_TARGET = "/";
static constexpr const char* const SERVER_MIME = "application/json";

using namespace std::string_literals;
using namespace std::chrono_literals;

namespace lynx {

	SyncHttpDictServer::SyncHttpDictServer(const std::string& host , uint16_t port)
		: mHost(host)
		, mPort(port)
		, mAcceptor(mContext)
		, mSignals(mContext)
		, mDictDao(host)
		, mStarted(false) {
		log::info(TAG, "Create server");
	}

	SyncHttpDictServer::~SyncHttpDictServer() {
		stop();
		log::info(TAG, "Destroy server");
	}

	bool SyncHttpDictServer::isStarted() const { return mStarted; }

	void SyncHttpDictServer::start() {
		log::info(TAG, "Start server");
		boost::system::error_code errorCode;

		startSignalHandler();

		mStarted = true;
		mDictDao.start();

		net::ip::tcp::endpoint endpoint(net::ip::tcp::v4(), mPort); /*net::ip::address::from_string(mHost)*/
		mAcceptor.open(endpoint.protocol());
		mAcceptor.set_option(net::ip::tcp::acceptor::reuse_address(true));
		mAcceptor.bind(endpoint);
		mAcceptor.listen();

		while (mStarted) {
			if (!mAcceptor.is_open()) {
				log::error(TAG, "Acceptor is already closed");
				return;
			}

			net::ip::tcp::socket socket(mContext);

			log::debug(TAG, "Ready accept client");
			mAcceptor.accept(socket, errorCode);

			if (errorCode) {
				log::error(TAG, "Can't accept client: %s", errorCode.message().c_str());
				return; //continue;
			}

			std::thread sessionThread(std::bind(&SyncHttpDictServer::processSession, this, std::move(socket)));
			sessionThread.detach();
		}
	}

	void SyncHttpDictServer::stop() {
		boost::system::error_code errorCode;

		mAcceptor.close(errorCode);

		if (errorCode) {
			log::error(TAG, "Can't close acceptor: %s", errorCode.message().c_str());
		}

		mStarted = false;
		mDictDao.stop();
		log::info(TAG, "Stop server");
	}

	void SyncHttpDictServer::startSignalHandler() {
		mSignals.add(SIGINT);
		mSignals.add(SIGTERM);
#if defined(SIGQUIT)
		mSignals.add(SIGQUIT);
#endif
		mSignals.async_wait([this](boost::system::error_code errorCode, int signalNumber) {
			if (!errorCode) {
				log::debug(TAG, "Received signal %d success", signalNumber);
			} else {
				log::error(TAG, "Can't wait signal %d: %s", signalNumber, errorCode.message().c_str());
			}

			stop();
		});
	}

	void SyncHttpDictServer::processSession(net::ip::tcp::socket& socket) {
		boost::system::error_code errorCode;
		beast::flat_buffer buffer;

		while (mStarted) {
			std::unique_ptr<http::message_generator> response;
			http::request<http::string_body> request;

			http::read(socket, buffer, request, errorCode);

			if (errorCode == http::error::end_of_stream) {
				log::error(TAG, "Received request with EOF");
				break;
			} else if (errorCode) {
				log::error(TAG, "Received request isn't correct: %s", errorCode.message().c_str());
				continue;
			}

			if (request.method() == http::verb::post && request.target() == "/post") {
				response = handlePostRequest(std::move(request));
			} else if (request.method() == http::verb::put && request.target() == "/put") {
				response = handlePutRequest(std::move(request));
			} else if (request.method() == http::verb::delete_ && request.target().starts_with("/delete/")) {
				response = handleDeleteRequest(std::move(request));
			} else if (request.method() == http::verb::put && request.target() == "/get") {
				response = handleGetAllRequest(std::move(request));
			} if (request.method() == http::verb::get && request.target().starts_with("/get/")) {
				response = handleGetByIdRequest(std::move(request));
			} else {
				log::error(TAG, "Received unknow http request");
			}

			const bool keepAlive = response->keep_alive();
			beast::write(socket, std::move(*response), errorCode);

			if (errorCode) {
				log::error(TAG, "Send response isn't correct: %s", errorCode.message().c_str());
				return;
			}

			if (!keepAlive) {
				log::error(TAG, "Made response hasn't keep alive");
				break;
			}

			std::this_thread::sleep_for(50ms);
		}

		socket.shutdown(net::ip::tcp::socket::shutdown_send, errorCode);
	}

	auto SyncHttpDictServer::handlePostRequest(http::request<http::string_body>&& request) -> std::unique_ptr<http::message_generator> {
		const uint32_t version = request.version();
		const bool keepAlive = request.keep_alive();

		if (!checkTarget(request.target())) {
			const std::string message = "Received illegal request-target";
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
					                http::status::bad_request, version, keepAlive));
		}

		boost::system::result<Word> remoteWord = mParser.deserializeFromText(request.body());

		if (remoteWord.has_error()) {
			const std::string message = format("Deserialize request word error %s",
											   remoteWord.error().message().c_str());
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
									http::status::internal_server_error, version, keepAlive));
		}

		boost::system::result<void> operationStatus = mDictDao.insert(remoteWord.value());

		if (operationStatus.has_value()) {
			const std::string message = "Handle POST/ request success";
			log::debug(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
									http::status::ok, version, keepAlive));
		} else {
			const auto message = format("Db insert word error %s",
					                    operationStatus.error().message().c_str());
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
									http::status::internal_server_error, version, keepAlive));
		}
	}

	auto SyncHttpDictServer::handlePutRequest(http::request<http::string_body>&& request) -> std::unique_ptr<http::message_generator> {
		const uint32_t version = request.version();
		const bool keepAlive = request.keep_alive();

		if (!checkTarget(request.target())) {
			const std::string message = "Received illegal request-target";
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
					                http::status::bad_request, version, keepAlive));
		}

		boost::system::result<Word> remoteWord = mParser.deserializeFromText(request.body());

		if (remoteWord.has_error()) {
			const std::string message = format("Deserialize request word error %s",
											   remoteWord.error().message().c_str());
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
									http::status::internal_server_error, version, keepAlive));
		}

		boost::system::result<void> operationStatus = mDictDao.update(remoteWord.value());

		if (operationStatus.has_value()) {
			const std::string message = "Handle PUT/ request success";
			log::debug(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
									http::status::ok, version, keepAlive));
		} else {
			const auto message = format("Db update word error %s",
										operationStatus.error().message().c_str());
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
									http::status::internal_server_error, version, keepAlive));
		}
	}

	auto SyncHttpDictServer::handleDeleteRequest(http::request<http::string_body>&& request) -> std::unique_ptr<http::message_generator> {
		const uint32_t version = request.version();
		const bool keepAlive = request.keep_alive();

		if (!checkTarget(request.target())) {
			const std::string message = "Received illegal request-target";
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
					                http::status::bad_request, version, keepAlive));
		}

		uint64_t wordId = 0;
		std::string remoteData = request.target().substr(request.target().find_last_of("/") + 1);
		auto remoteWordId = std::from_chars(remoteData.data(), remoteData.data() + remoteData.size(), wordId);

		if (remoteWordId.ec != std::errc{}) {
			const std::string message = format("Parse word id error: %s",
											   std::make_error_code(remoteWordId.ec).message().c_str());
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
									http::status::internal_server_error, version, keepAlive));
		}

		boost::system::result<void> operationStatus = mDictDao.remove(wordId);

		if (operationStatus.has_value()) {
			const std::string message = format("Handle DELETE/%u/ request success", wordId);
			log::debug(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
									http::status::ok, version, keepAlive));
		} else {
			const auto message = format("Db delete word error %s",
										operationStatus.error().message().c_str());
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
									http::status::internal_server_error, version, keepAlive));
		}
	}

	auto SyncHttpDictServer::handleGetByIdRequest(http::request<http::string_body>&& request) -> std::unique_ptr<http::message_generator> {
		const uint32_t version = request.version();
		const bool keepAlive = request.keep_alive();

		if (!checkTarget(request.target())) {
			const std::string message = "Received illegal request-target";
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
					                http::status::bad_request, version, keepAlive));
		}

		uint64_t wordId = 0;
		std::string remoteData = request.target().substr(request.target().find_last_of("/") + 1);
		auto remoteWordId = std::from_chars(remoteData.data(), remoteData.data() + remoteData.size(), wordId);

		if (remoteWordId.ec != std::errc{}) {
			const std::string message = format("Parse word id error: %s",
											   std::make_error_code(remoteWordId.ec).message().c_str());
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
									http::status::internal_server_error, version, keepAlive));
		}

		boost::system::result<Word> localWord = mDictDao.getById(wordId);

		if (localWord.has_error()) {
			const std::string message = format("Db get word by id error: %s",
											   localWord.error().message().c_str());
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
									http::status::internal_server_error, version, keepAlive));
		}

		boost::system::result<std::string> localData = mParser.serializeToText(*localWord);

		if (localData.has_value()) {
			const std::string message = format("Handle GET/%u/ request success", wordId);
			log::debug(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(*localData,
									http::status::ok, version, keepAlive));
		} else {
			const auto message = format("Serialize word error: %s",
										localData.error().message().c_str());
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
									http::status::internal_server_error, version, keepAlive));
		}
	}

	auto SyncHttpDictServer::handleGetAllRequest(http::request<http::string_body>&& request) -> std::unique_ptr<http::message_generator> {
		const uint32_t version = request.version();
		const bool keepAlive = request.keep_alive();

		if (!checkTarget(request.target())) {
			const std::string message = "Received illegal request-target";
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
					                http::status::bad_request, version, keepAlive));
		}

		boost::system::result<std::vector<Word>> localWords = mDictDao.getAll();

		if (localWords.has_error()) {
			const std::string message = format("Db get all words error: %s",
											   localWords.error().message().c_str());
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
									http::status::internal_server_error, version, keepAlive));
		}


		boost::system::result<std::string> localData = mParser.serializeWordsToText(*localWords);

		if (localData.has_value()) {
			const std::string message = format("Handle GET/ request success");
			log::debug(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(*localData,
									http::status::ok, version, keepAlive));
		} else {
			const auto message = format("Serialize words error: %s",
										localData.error().message().c_str());
			log::error(TAG, message);
			return std::make_unique<http::message_generator>(prepareResponse(message,
									http::status::internal_server_error, version, keepAlive));
		}
	}

	auto SyncHttpDictServer::prepareResponse(const std::string& body, http::status status, uint32_t version, bool keepAlive)
		-> http::response<http::string_body> {
		http::response<http::string_body> response{status, version};
		response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		response.set(http::field::content_type, SERVER_MIME);
		response.keep_alive(keepAlive);
		response.body() = body;
		response.prepare_payload();

		return response;
	}

	bool SyncHttpDictServer::checkTarget(boost::core::string_view target) const {
		return !target.empty() || target[0] == '/' || target.find("..") == boost::core::string_view::npos;
	}
}




