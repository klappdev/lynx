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

#include "net/SyncDictClient.hpp"
#include "net/DictCommand.hpp"
#include "net/NetworkUtils.hpp"
#include "logging/Logging.hpp"

#include <thread>

#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

static constexpr const char* const TAG = "SyncDictClient";

using namespace std::string_literals;
using namespace std::chrono_literals;

namespace lynx {

	SyncDictClient::SyncDictClient(const std::string& host, uint64_t port)
		: mSocket(mContext)
		, mEndpoint(boost::asio::ip::address::from_string(host), port)
		, mStarted(false) {
		log::info(TAG, "Create client");
	}

	SyncDictClient::~SyncDictClient() {
		if (mSocket.is_open()) {
			mSocket.close();
		}
		mStarted = false;

		log::info(TAG, "Destroy client");
	}

	bool SyncDictClient::isStarted() const { return mStarted; }

	void SyncDictClient::start() {
		log::info(TAG, "Start client");

		boost::system::error_code errorCode;
		mSocket.connect(mEndpoint, errorCode);

		if (!errorCode) {
			mStarted = true;
		} else {
			log::error(TAG, "Can't connect to server: %s", errorCode.message().c_str());
		}
	}

	void SyncDictClient::stop() {
		mStarted = false;
		log::info(TAG, "Stop client");
	}

	void SyncDictClient::performQuit() {
		boost::system::error_code errorCode;
		boost::asio::write(mSocket, boost::asio::buffer(QUIT_COMMAND + "\n"s), errorCode);

		if (!errorCode) {
			log::info(TAG, "Send message %s successfully", QUIT_COMMAND);
		} else {
			log::error(TAG, "Can't send message %s: %s", QUIT_COMMAND, errorCode.message().c_str());
		}

		std::this_thread::sleep_for(1s);
	}

	void SyncDictClient::performInsert(const Word& word) {
		boost::system::error_code errorCode;
		boost::asio::streambuf remoteBuffer;

		boost::system::result<std::string> localData = mParser.serializeToText(word);

		if (localData.has_error()) {
			log::error(TAG, "Serialize word error: %s", errorCode.message().c_str());
			return;
		}

		boost::asio::write(mSocket, boost::asio::buffer(INSERT_COMMAND + localData.value() + "\n"), errorCode);

		if (errorCode) {
			log::error(TAG, "Can't send message %s: %s", INSERT_COMMAND, errorCode.message().c_str());
			return;
		}

		errorCode.clear();
		boost::asio::read_until(mSocket, remoteBuffer, "\n", errorCode);

		std::string remoteData = toString(remoteBuffer);

		if (!errorCode && remoteData.starts_with(INSERT_COMMAND)) {
			log::debug(TAG, "Receive message: %s", remoteData.c_str());
		} else {
			log::error(TAG, "Receive message isn't correct: %s", errorCode.message().c_str());
		}
	}

	void SyncDictClient::performUpdate(const Word& word) {
		boost::system::error_code errorCode;
		boost::asio::streambuf remoteBuffer;

		boost::system::result<std::string> localData = mParser.serializeToText(word);

		if (localData.has_error()) {
			log::error(TAG, "Serialize word error: %s", errorCode.message().c_str());
			return;
		}

		boost::asio::write(mSocket, boost::asio::buffer(UPDATE_COMMAND + localData.value() + "\n"), errorCode);

		if (errorCode) {
			log::error(TAG, "Can't send message %s: %s", UPDATE_COMMAND, errorCode.message().c_str());
			return;
		}

		errorCode.clear();
		boost::asio::read_until(mSocket, remoteBuffer, "\n", errorCode);

		std::string remoteData = toString(remoteBuffer);

		if (!errorCode && remoteData.starts_with(UPDATE_COMMAND)) {
			log::debug(TAG, "Receive message: %s", remoteData.c_str());
		} else {
			log::error(TAG, "Receive message isn't correct: %s", errorCode.message().c_str());
		}
	}

	void SyncDictClient::performDelete(uint64_t id) {
		boost::system::error_code errorCode;
		boost::asio::streambuf remoteBuffer;

		boost::asio::write(mSocket, boost::asio::buffer(DELETE_COMMAND + std::to_string(id) + "\n"), errorCode);

		if (errorCode) {
			log::error(TAG, "Can't send message %s: %s", DELETE_COMMAND, errorCode.message().c_str());
			return;
		}

		errorCode.clear();
		boost::asio::read_until(mSocket, remoteBuffer, "\n", errorCode);

		std::string remoteData = toString(remoteBuffer);

		if (!errorCode && remoteData.starts_with(DELETE_COMMAND)) {
			log::debug(TAG, "Receive message: %s", remoteData.c_str());
		} else {
			log::error(TAG, "Receive message isn't correct: %s", errorCode.message().c_str());
		}
	}

	auto SyncDictClient::performGetById(uint64_t id) -> Word {
		boost::system::error_code errorCode;
		boost::asio::streambuf remoteBuffer;

		boost::asio::write(mSocket, boost::asio::buffer(GET_BY_ID_COMMAND + std::to_string(id) + "\n"), errorCode);

		if (errorCode) {
			log::error(TAG, "Can't send message %s: %s", GET_BY_ID_COMMAND, errorCode.message().c_str());
			return {};
		}

		errorCode.clear();
		boost::asio::read_until(mSocket, remoteBuffer, "\n", errorCode);

		std::string remoteData = toString(remoteBuffer);

		if (!errorCode && remoteData.starts_with(GET_BY_ID_COMMAND)) {
			log::debug(TAG, "Receive message: %s", remoteData.c_str());
		} else {
			log::error(TAG, "Receive message isn't correct: %s", errorCode.message().c_str());
		}

		std::string rawRemoteData = remoteData.substr(std::strlen(GET_BY_ID_COMMAND));
		boost::system::result<Word> remoteWord = mParser.deserializeFromText(rawRemoteData);

		if (remoteWord.has_value()) {
			return *remoteWord;
		} else {
			log::error(TAG, "Deserialize word error: %s", remoteWord.error().message().c_str());
			return {};
		}
	}

	auto SyncDictClient::performGetAll() -> std::vector<Word> {
		boost::system::error_code errorCode;
		boost::asio::streambuf remoteBuffer;

		boost::asio::write(mSocket, boost::asio::buffer(GET_ALL_COMMAND + "\n"s), errorCode);

		if (errorCode) {
			log::error(TAG, "Can't send message %s: %s", GET_ALL_COMMAND, errorCode.message().c_str());
			return {};
		}

		errorCode.clear();
		boost::asio::read_until(mSocket, remoteBuffer, "\n", errorCode);

		std::string remoteData = toString(remoteBuffer);

		if (!errorCode && remoteData.starts_with(GET_ALL_COMMAND)) {
			log::debug(TAG, "Receive message: %s", remoteData.c_str());
		} else {
			log::error(TAG, "Receive message isn't correct: %s", errorCode.message().c_str());
		}

		std::string rawRemoteData = remoteData.substr(std::strlen(GET_ALL_COMMAND));
		boost::system::result<std::vector<Word>> remoteWords = mParser.deserializeWordsFromText(rawRemoteData);

		if (remoteWords.has_value()) {
			return *remoteWords;
		} else {
			log::error(TAG, "Deserialize words error: %s", remoteWords.error().message().c_str());
			return {};
		}
	}
}
