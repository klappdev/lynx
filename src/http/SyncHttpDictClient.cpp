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

#include "http/SyncHttpDictClient.hpp"
#include "logging/Logging.hpp"

#include <boost/beast/version.hpp>

static constexpr const char* const TAG = "SyncHttpDictClient";

namespace lynx {

	SyncHttpDictClient::SyncHttpDictClient(const std::string& host, uint16_t port)
		: mHost(host)
		, mPort(port)
		, mStream(mContext)
		, mStarted(false) {
		log::info(TAG, "Create client");
	}

	SyncHttpDictClient::~SyncHttpDictClient() {
		auto& socket = mStream.socket();

		if (socket.is_open()) {
			socket.shutdown(net::ip::tcp::socket::shutdown_both);
		}

		log::info(TAG, "Destroy client");
	}

	bool SyncHttpDictClient::isStarted() const { return mStarted; }

	void SyncHttpDictClient::start() {
		log::info(TAG, "Start client");

		boost::system::error_code errorCode;

		net::ip::tcp::resolver resolver(mContext);
		auto endpoints = resolver.resolve(mHost, std::to_string(mPort), errorCode);

		if (errorCode) {
			log::error(TAG, "Can't resolve host of http server %s: %s", mHost.c_str(), errorCode.message().c_str());
			return;
		}

		mStream.connect(endpoints, errorCode);

		if (!errorCode) {
			mStarted = true;
			log::debug(TAG, "Connect to http server %s success", mHost.c_str());
		} else {
			log::error(TAG, "Can't connect to http server %s: %s", mHost.c_str(), errorCode.message().c_str());
		}
	}

	void SyncHttpDictClient::stop() {
		mStarted = false;
		log::info(TAG, "Stop client");
	}

	void SyncHttpDictClient::performPost(const Word& word) {
		boost::system::error_code errorCode;
		const std::string verbRequest = http::to_string(http::verb::post);

		boost::system::result<std::string> localData = mParser.serializeToText(word);

		if (localData.has_error()) {
			log::error(TAG, "Serialize word error: %s", errorCode.message().c_str());
			return;
		}

		http::request<http::string_body> request = prepareRequest(http::verb::post, "/post", localData.value());
		http::write(mStream, request, errorCode);

		if (!errorCode) {
			log::debug(TAG, "Write request %s success", verbRequest.c_str());
		} else {
			log::error(TAG, "Can't write request %s: %s", verbRequest.c_str(), errorCode.message().c_str());
			return;
		}

		beast::flat_buffer buffer;
		http::response<http::dynamic_body> response;
		http::read(mStream, buffer, response, errorCode);

		if (!errorCode) {
			log::debug(TAG, "Read response %s success", verbRequest.c_str());
		} else {
			log::error(TAG, "Can't read response %s: %s", verbRequest.c_str(), errorCode.message().c_str());
		}
	}

	void SyncHttpDictClient::performPut(const Word& word) {
		boost::system::error_code errorCode;
		const std::string verbRequest = http::to_string(http::verb::put);

		boost::system::result<std::string> localData = mParser.serializeToText(word);

		if (localData.has_error()) {
			log::error(TAG, "Serialize word error: %s", errorCode.message().c_str());
			return;
		}

		http::request<http::string_body> request = prepareRequest(http::verb::put, "/put", localData.value());
		http::write(mStream, request, errorCode);

		if (!errorCode) {
			log::debug(TAG, "Write request %s success", verbRequest.c_str());
		} else {
			log::error(TAG, "Can't write request %s: %s", verbRequest.c_str(), errorCode.message().c_str());
			return;
		}

		beast::flat_buffer buffer;
		http::response<http::dynamic_body> response;
		http::read(mStream, buffer, response, errorCode);

		if (!errorCode) {
			log::debug(TAG, "Read response %s success", verbRequest.c_str());
		} else {
			log::error(TAG, "Can't read response %s: %s", verbRequest.c_str(), errorCode.message().c_str());
		}
	}

	void SyncHttpDictClient::performDelete(uint64_t id) {
		boost::system::error_code errorCode;
		const std::string verbRequest = http::to_string(http::verb::delete_);

		http::request<http::string_body> request = prepareRequest(http::verb::delete_, "/delete/" + std::to_string(id));
		http::write(mStream, request, errorCode);

		if (!errorCode) {
			log::debug(TAG, "Write request %s/%lu success", verbRequest.c_str(), id);
		} else {
			log::error(TAG, "Can't write request %s/%lu: %s", verbRequest.c_str(), id, errorCode.message().c_str());
			return;
		}

		beast::flat_buffer buffer;
		http::response<http::dynamic_body> response;
		http::read(mStream, buffer, response, errorCode);

		if (!errorCode) {
			log::debug(TAG, "Read response %s/%lu success", verbRequest.c_str(), id);
		} else {
			log::error(TAG, "Can't read response %s/%lu: %s", verbRequest.c_str(), id, errorCode.message().c_str());
		}
	}

	auto SyncHttpDictClient::performGet(uint64_t id) -> Word {
		boost::system::error_code errorCode;
		const std::string verbRequest = http::to_string(http::verb::get);

		http::request<http::string_body> request = prepareRequest(http::verb::get, "/get/" + std::to_string(id));
		http::write(mStream, request, errorCode);

		if (!errorCode) {
			log::debug(TAG, "Write request %s/%lu success", verbRequest.c_str(), id);
		} else {
			log::error(TAG, "Can't write request %s/%lu: %s", verbRequest.c_str(), id, errorCode.message().c_str());
			return {};
		}

		beast::flat_buffer buffer;
		http::response<http::dynamic_body> response;
		http::read(mStream, buffer, response, errorCode);

		if (!errorCode) {
			log::debug(TAG, "Read response %s/%lu success", verbRequest.c_str(), id);
		} else {
			log::error(TAG, "Can't read response %s/%lu: %s", verbRequest.c_str(), id, errorCode.message().c_str());
			return {};
		}

		std::string remoteData = beast::buffers_to_string(response.body().data());

		boost::system::result<Word> remoteWord = mParser.deserializeFromText(remoteData);

		if (remoteWord.has_value()) {
			return *remoteWord;
		} else {
			log::error(TAG, "Deserialize word error %s", remoteWord.error().message().c_str());
			return {};
		}
	}

	auto SyncHttpDictClient::performGet() -> std::vector<Word> {
		boost::system::error_code errorCode;
		const std::string verbRequest = http::to_string(http::verb::get);

		http::request<http::string_body> request = prepareRequest(http::verb::get, "/get");
		http::write(mStream, request, errorCode);

		if (!errorCode) {
			log::debug(TAG, "Write request %s success", verbRequest.c_str());
		} else {
			log::error(TAG, "Can't write request %s: %s", verbRequest.c_str(), errorCode.message().c_str());
			return {};
		}

		beast::flat_buffer buffer;
		http::response<http::dynamic_body> response;
		http::read(mStream, buffer, response, errorCode);

		if (!errorCode) {
			log::debug(TAG, "Read response %s success", verbRequest.c_str());
		} else {
			log::error(TAG, "Can't read response %s: %s", verbRequest.c_str(), errorCode.message().c_str());
			return {};
		}

		std::string remoteData = beast::buffers_to_string(response.body().data());

		boost::system::result<std::vector<Word>> remoteWords = mParser.deserializeWordsFromText(remoteData);

		if (remoteWords.has_value()) {
			return *remoteWords;
		} else {
			log::error(TAG, "Deserialize words error %s", remoteWords.error().message().c_str());
			return {};
		}
	}

	auto SyncHttpDictClient::prepareRequest(http::verb method, const std::string& target, const std::string& body)
		-> http::request<http::string_body> {
		http::request<http::string_body> request;
		request.set(http::field::host, mHost);
		request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		request.target(target);
		request.method(method);
		request.body() = body;
		request.prepare_payload();

		return request;
	}

}




