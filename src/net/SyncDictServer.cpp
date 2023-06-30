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

#include "net/SyncDictServer.hpp"
#include "net/DictCommand.hpp"
#include "net/NetworkUtils.hpp"
#include "logging/Logging.hpp"

#include <thread>
#include <charconv>

#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

static constexpr const char* const TAG = "SyncDictServer";

using namespace std::string_literals;
using namespace std::chrono_literals;

namespace lynx {

	SyncDictServer::SyncDictServer(const std::string& host, uint64_t port)
		: mSocket(mContext)
		, mEndpoint(boost::asio::ip::tcp::v4(), port)
		, mAcceptor(mContext, mEndpoint)
		, dictDao(host)
		, mStarted(false) {
		log::info(TAG, "Create server");
	}

	SyncDictServer::~SyncDictServer() {
		if (mSocket.is_open()) {
			mSocket.close();
		}
		mStarted = false;

		log::info(TAG, "Destroy server");
	}

	bool SyncDictServer::isStarted() const { return mStarted; }

	void SyncDictServer::start() {
		log::info(TAG, "Start server");

		boost::system::error_code errorCode;
		mAcceptor.accept(mSocket, errorCode);

		if (!errorCode) {
			mStarted = true;
			dictDao.start();
			processMessages();
		} else {
			log::error(TAG, "Can't accept client: %s", errorCode.message().c_str());
		}
	}

	void SyncDictServer::stop() {
		mStarted = false;
		dictDao.stop();
		log::info(TAG, "Stop server");
	}

	void SyncDictServer::processMessages() {
		boost::system::error_code errorCode;
		boost::asio::streambuf remoteBuffer;

		while (mStarted) {
			boost::asio::read_until(mSocket, remoteBuffer, "\n", errorCode);

			if (errorCode) {
				log::error(TAG, "Receive data isn't correct: %s", errorCode.message().c_str());
				return;
			}

			std::string remoteData = toString(remoteBuffer);

			if (remoteData.starts_with(INSERT_COMMAND)) {
				processInsert(remoteData);
			} else if (remoteData.starts_with(UPDATE_COMMAND)) {
				processUpdate(remoteData);
			} else if (remoteData.starts_with(DELETE_COMMAND)) {
				processDelete(remoteData);
			} else if (remoteData.starts_with(GET_BY_ID_COMMAND)) {
				processGetById(remoteData);
			} else if (remoteData.starts_with(GET_ALL_COMMAND)) {
				processGetAll(remoteData);
			} else if (remoteData.starts_with(QUIT_COMMAND)) {
				log::error(TAG, "Process %s message", QUIT_COMMAND);
				mStarted = false;
			} else {
				log::error(TAG, "Process unknown message");
			}

			std::this_thread::sleep_for(500ms);
		}
	}

	void SyncDictServer::processInsert(const std::string& message) {
		log::debug(TAG, "Process %s message", INSERT_COMMAND);

		std::string remoteData = message.substr(std::strlen(INSERT_COMMAND));

		boost::system::result<Word> remoteWord = mParser.deserializeFromText(remoteData);

		if (remoteWord.has_error()) {
			boost::asio::write(mSocket, boost::asio::buffer("Deserialize word error"s + "\n"));
			log::error(TAG, "Deserialize word error: %s", remoteWord.error().message().c_str());
			return;
		}

		boost::system::result<void> operationStatus = dictDao.insert(remoteWord.value());

		if (operationStatus.has_error()) {
			boost::asio::write(mSocket, boost::asio::buffer("Db insert word error"s + "\n"));
			log::error(TAG, "Db insert word error: %s", remoteWord.error().message().c_str());
			return;
		}

		boost::system::error_code errorCode;
		boost::asio::write(mSocket, boost::asio::buffer(INSERT_COMMAND + " processed success"s + "\n"), errorCode);

		if (!errorCode) {
			log::info(TAG, "Send message: %s", INSERT_COMMAND);
		} else {
			log::error(TAG, "Can't send %s message: %s", INSERT_COMMAND, errorCode.message().c_str());
		}
	}

	void SyncDictServer::processUpdate(const std::string& message) {
		log::debug(TAG, "Process %s message", UPDATE_COMMAND);

		std::string remoteData = message.substr(std::strlen(UPDATE_COMMAND));

		boost::system::result<Word> remoteWord = mParser.deserializeFromText(remoteData);

		if (remoteWord.has_error()) {
			boost::asio::write(mSocket, boost::asio::buffer("Deserialize word error"s + "\n"));
			log::error(TAG, "Deserialize word error: %s", remoteWord.error().message().c_str());
			return;
		}

		boost::system::result<void> operationStatus = dictDao.update(remoteWord.value());

		if (operationStatus.has_error()) {
			boost::asio::write(mSocket, boost::asio::buffer("Db update word error"s + "\n"));
			log::error(TAG, "Db update word error: %s", remoteWord.error().message().c_str());
			return;
		}

		boost::system::error_code errorCode;
		boost::asio::write(mSocket, boost::asio::buffer(UPDATE_COMMAND + " processed success"s + "\n"), errorCode);

		if (!errorCode) {
			log::info(TAG, "Send message: %s", UPDATE_COMMAND);
		} else {
			log::error(TAG, "Can't send %s message: %s", UPDATE_COMMAND, errorCode.message().c_str());
		}
	}

	void SyncDictServer::processDelete(const std::string& message) {
		log::debug(TAG, "Process %s message", DELETE_COMMAND);

		uint64_t wordId = 0;
		std::string remoteData = message.substr(std::strlen(DELETE_COMMAND));
		auto remoteWordId = std::from_chars(remoteData.data(), remoteData.data() + remoteData.size(), wordId);

		if (remoteWordId.ec != std::errc{}) {
			boost::asio::write(mSocket, boost::asio::buffer("Parse word id error"s + "\n"));
			log::error(TAG, "Parse word id error: %s", std::make_error_code(remoteWordId.ec).message().c_str());
			return;
		}

		boost::system::result<void> operationStatus = dictDao.remove(wordId);

		if (operationStatus.has_error()) {
			boost::asio::write(mSocket, boost::asio::buffer("Db delete word error"s + "\n"));
			log::error(TAG, "Db delete word error: %s", operationStatus.error().message().c_str());
			return;
		}

		boost::system::error_code errorCode;
		boost::asio::write(mSocket, boost::asio::buffer(DELETE_COMMAND + " processed success"s + "\n"), errorCode);

		if (!errorCode) {
			log::info(TAG, "Send message: %s", DELETE_COMMAND);
		} else {
			log::error(TAG, "Can't send %s message: %s", DELETE_COMMAND, errorCode.message().c_str());
		}
	}

	void SyncDictServer::processGetById(const std::string& message) {
		log::debug(TAG, "Process %s message", GET_BY_ID_COMMAND);

		uint64_t wordId = 0;
		std::string remoteData = message.substr(std::strlen(GET_BY_ID_COMMAND));
		auto remoteWordId = std::from_chars(remoteData.data(), remoteData.data() + remoteData.size(), wordId);

		if (remoteWordId.ec != std::errc{}) {
			boost::asio::write(mSocket, boost::asio::buffer("Parse word id error"s + "\n"));
			log::error(TAG, "Parse word id error: %s", std::make_error_code(remoteWordId.ec).message().c_str());
			return;
		}

		boost::system::result<Word> localWord = dictDao.getById(wordId);

		if (localWord.has_error()) {
			boost::asio::write(mSocket, boost::asio::buffer("Db get word by id error"s + "\n"));
			log::error(TAG, "Db get word by id error: %s", localWord.error().message().c_str());
			return;
		}

		boost::system::result<std::string> localData = mParser.serializeToText(*localWord);

		if (localData.has_error()) {
			boost::asio::write(mSocket, boost::asio::buffer("Serialize word error"s + "\n"));
			log::error(TAG, "Serialize word error: %s", localData.error().message().c_str());
			return;
		}

		boost::system::error_code errorCode;
		boost::asio::write(mSocket, boost::asio::buffer(GET_BY_ID_COMMAND + localData.value() + "\n"), errorCode);

		if (!errorCode) {
			log::info(TAG, "Send message: %s", GET_BY_ID_COMMAND);
		} else {
			log::error(TAG, "Can't send %s message: %s", GET_BY_ID_COMMAND, errorCode.message().c_str());
		}
	}

	void SyncDictServer::processGetAll(const std::string& message) {
		log::debug(TAG, "Process %s message", GET_ALL_COMMAND);

		boost::system::result<std::vector<Word>> localWords = dictDao.getAll();

		if (localWords.has_error()) {
			boost::asio::write(mSocket, boost::asio::buffer("Db get all words error"s + "\n"));
			log::error(TAG, "Db get all words error: %s", localWords.error().message().c_str());
			return;
		}

		boost::system::result<std::string> localData = mParser.serializeWordsToText(*localWords);

		if (localData.has_error()) {
			boost::asio::write(mSocket, boost::asio::buffer("Serialize words error"s + "\n"));
			log::error(TAG, "Serialize words error: %s", localData.error().message().c_str());
			return;
		}

		boost::system::error_code errorCode;
		boost::asio::write(mSocket, boost::asio::buffer(GET_ALL_COMMAND + localData.value() + "\n"), errorCode);

		if (!errorCode) {
			log::info(TAG, "Send message: %s", GET_ALL_COMMAND);
		} else {
			log::error(TAG, "Can't send %s message: %s", GET_ALL_COMMAND, errorCode.message().c_str());
		}
	}
}
