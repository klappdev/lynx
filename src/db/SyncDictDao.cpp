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

#include "db/SyncDictDao.hpp"
#include "logging/Logging.hpp"

#include <boost/url/parse.hpp>

static constexpr const char* const TAG = "SyncDictDao";
static constexpr const char* const DATABASE_NAME = "dictionary";
static constexpr const char* const WORD_TABLE_NAME = "word";
static constexpr const char* const WORD_IMAGE_TABLE_NAME = "word_image";
static constexpr const char* const USER_NAME = "user";
static constexpr const char* const PASSWORD = "pass";

/**
 * #XXX: Disable from autostart:
 * $ systemctl disable mysql
 *
 * #XXX: Start/stop mysql:
 * $ service mysql stop
 * $ service mysql start
 *
 * #XXX Start mysql client:
 * $ mysql -u user -p
 *
 * #XXX Create database:
 * > DROP DATABASE IF EXISTS dictionary;
 * > CREATE DATABASE dictionary;
 *
 * #XXX: Show databases:
 * > SHOW DATABASES;
 * > USE dictionary;
 *
 * #XXX: Show tables:
 * > SHOW TABLES;
 *
 * #XXX: Show info tables:
 * > DESCRIBE word;
 * > DESCRIBE word_image;
 *
 * #XXX: Show users and tables:
 * > SELECT user FROM mysql.user;
 * > SELECT * FROM word;
 * > SELECT * FROM word_image;
 */

namespace lynx {

	SyncDictDao::SyncDictDao(const std::string& host)
		: mHost(host)
		, mConnection(nullptr)
		, mLastWordId(0)
		, mLastWordImageId(0)
        , mStarted(false) {
		log::info(TAG, "Create dict dao");
	}

	SyncDictDao::~SyncDictDao() {
		boost::system::error_code errorCode;
		boost::mysql::diagnostics serverErrorCode;

		mConnection->close(errorCode, serverErrorCode);

		if (errorCode) {
			log::error(TAG, "Connection to db server close with error: %s, %s",
					   errorCode.message().c_str(), std::string(serverErrorCode.server_message()).c_str());
		}

		mStarted = false;
		log::info(TAG, "Destroy dict dao");
	}

	bool SyncDictDao::isStarted() const { return mStarted; }

	auto SyncDictDao::getLastWordId() const -> uint64_t { return mLastWordId; }
	auto SyncDictDao::getLastWordImageId() const -> uint64_t { return mLastWordImageId; }

	void SyncDictDao::start() {
		log::info(TAG, "Connect to %s tables", DATABASE_NAME);

		boost::system::error_code errorCode;
		boost::mysql::diagnostics serverErrorCode;

		net::ip::tcp::resolver resolver(mContext.get_executor());
		auto endpoints = resolver.resolve(mHost, boost::mysql::default_port_string, errorCode);

		if (errorCode) {
			log::error(TAG, "Can't resolve host of db server %s: %s", mHost.c_str(), errorCode.message().c_str());
			return;
		}

		net::ssl::context sslContext(net::ssl::context::tls_client);
		mConnection = std::make_unique<db::tcp_ssl_connection>(mContext, sslContext);

		db::handshake_params parameters(USER_NAME, PASSWORD, DATABASE_NAME);
		mConnection->connect(*endpoints.begin(), parameters, errorCode, serverErrorCode);

		if (errorCode) {
			log::error(TAG, "Can't connect to db server: %s, %s",
					   errorCode.message().c_str(), std::string(serverErrorCode.server_message()).c_str());
			return;
		}

		createTables();
	}

	void SyncDictDao::stop() {
		mStarted = false;
		log::info(TAG, "Disconnect from %s tables", DATABASE_NAME);
	}

	void SyncDictDao::createTables() {
		boost::system::error_code errorCode;
		db::diagnostics serverErrorCode;
		db::results result;

		mConnection->query(R"xxx(
			CREATE TABLE IF NOT EXISTS word_image (
				id INT PRIMARY KEY AUTO_INCREMENT,
				url TEXT,
				width INT NOT NULL,
				height INT NOT NULL
			);
		)xxx",
		result, errorCode, serverErrorCode);

		if (!errorCode) {
			log::debug(TAG, "Create %s table success", WORD_IMAGE_TABLE_NAME);
		} else {
			log::error(TAG, "Can't create %s table: %s, %s", WORD_IMAGE_TABLE_NAME,
					   errorCode.message().c_str(), std::string(serverErrorCode.server_message()).c_str());
			return;
		}

		mConnection->query(R"xxx(
			CREATE TABLE IF NOT EXISTS word (
				id INT PRIMARY KEY AUTO_INCREMENT,
				id_image INT NOT NULL,
				name TEXT,
				`index` INT NOT NULL,
				type ENUM('NOUN', 'ADJECTIVE', 'VERB', 'ADVERB'),
				FOREIGN KEY (id_image) REFERENCES word_image(id)
			);
		)xxx",
		result, errorCode, serverErrorCode);

		if (!errorCode) {
			log::debug(TAG, "Create %s table success", WORD_TABLE_NAME);
		} else {
			log::error(TAG, "Can't create %s table: %s, %s", WORD_TABLE_NAME,
					   errorCode.message().c_str(), std::string(serverErrorCode.server_message()).c_str());
			return;
		}
	}

	void SyncDictDao::truncateTables() {
		boost::system::error_code errorCode;
		db::diagnostics serverErrorCode;
		db::results result;

		mConnection->query("START TRANSACTION", result);
		mConnection->query("SET FOREIGN_KEY_CHECKS = 0", result);

		mConnection->query("TRUNCATE word", result, errorCode, serverErrorCode);

		if (!errorCode) {
			log::debug(TAG, "Truncate %s table success", WORD_TABLE_NAME);
		} else {
			log::error(TAG, "Can't truncate %s table: %s, %s", WORD_TABLE_NAME,
					   errorCode.message().c_str(), std::string(serverErrorCode.server_message()).c_str());
			return;
		}

		mConnection->query("TRUNCATE word_image", result, errorCode, serverErrorCode);

		if (!errorCode) {
			log::debug(TAG, "Truncate %s table success", WORD_IMAGE_TABLE_NAME);
		} else {
			log::error(TAG, "Can't truncate %s table: %s, %s", WORD_IMAGE_TABLE_NAME,
					   errorCode.message().c_str(), std::string(serverErrorCode.server_message()).c_str());
			return;
		}

		mConnection->query("SET FOREIGN_KEY_CHECKS = 1", result);
		mConnection->query("COMMIT", result);
	}

	auto SyncDictDao::insert(const Word& word) -> boost::system::result<void> {
		boost::system::error_code errorCode;
		db::diagnostics serverErrorCode;
		db::results result;

		mConnection->query("START TRANSACTION", result);

		db::statement statement = mConnection->prepare_statement(
			"INSERT INTO word_image (url, width, height) VALUES (?, ?, ?)"
		);
		auto wordImageParameters = std::make_tuple(std::string(word.image.url.c_str()),
												   word.image.width, word.image.height);
		mConnection->execute_statement(statement, wordImageParameters, result, errorCode, serverErrorCode);

		if (!errorCode) {
			mLastWordImageId = result.last_insert_id();
		} else {
			log::error(TAG, "Can't insert word image in table: %s, %s",
					   errorCode.message().c_str(), std::string(serverErrorCode.server_message()).c_str());
			return errorCode;
		}

		statement = mConnection->prepare_statement(
			"INSERT INTO word (id_image, name, `index`, type) VALUES (?, ?, ?, ?)"
		);
		auto wordParameters = std::make_tuple(mLastWordImageId, word.name, word.index,
											  boost::describe::enum_to_string(word.type, "NOUN"));
		mConnection->execute_statement(statement, wordParameters, result, errorCode, serverErrorCode);

		if (!errorCode) {
			mLastWordId = result.last_insert_id();
		} else {
			log::error(TAG, "Can't insert word in table: %s, %s",
					   errorCode.message().c_str(), std::string(serverErrorCode.server_message()).c_str());
			return errorCode;
		}

		mConnection->query("COMMIT", result);

		return {};
	}

	auto SyncDictDao::update(const Word& word) -> boost::system::result<void> {
		boost::system::error_code errorCode;
		db::diagnostics serverErrorCode;
		db::results result;

		mConnection->query("START TRANSACTION", result);

		db::statement statement = mConnection->prepare_statement(
			"UPDATE word_image SET url=?, width=?, height=? WHERE id=?"
		);
		auto wordImageParameters = std::make_tuple(std::string(word.image.url.c_str()),
												   word.image.width, word.image.height, word.image.id);
		mConnection->execute_statement(statement, wordImageParameters, result, errorCode, serverErrorCode);

		if (errorCode) {
			log::error(TAG, "Can't update word image in table: %s, %s",
					   errorCode.message().c_str(), std::string(serverErrorCode.server_message()).c_str());
			return errorCode;
		}

		statement = mConnection->prepare_statement(
			"UPDATE word SET id_image=?, name=?, `index`=?, type=? WHERE id=?"
		);
		auto wordParameters = std::make_tuple(word.image.id, word.name, word.index,
											  boost::describe::enum_to_string(word.type, "NOUN"), word.id);
		mConnection->execute_statement(statement, wordParameters, result, errorCode, serverErrorCode);

		if (errorCode) {
			log::error(TAG, "Can't update word in table: %s, %s",
					   errorCode.message().c_str(), std::string(serverErrorCode.server_message()).c_str());
			return errorCode;
		}

		mConnection->query("COMMIT", result);

		return {};
	}

	auto SyncDictDao::remove(uint64_t id) -> boost::system::result<void> {
		boost::system::error_code errorCode;
		db::diagnostics serverErrorCode;
		db::results result;

		mConnection->query("START TRANSACTION", result);
		mConnection->query("SET FOREIGN_KEY_CHECKS = 0", result);

		db::statement statement = mConnection->prepare_statement(
			"DELETE FROM word_image WHERE id IN (SELECT id_image FROM word WHERE id=?)"
		);
		mConnection->execute_statement(statement, std::make_tuple(id), result, errorCode, serverErrorCode);

		if (errorCode) {
			log::error(TAG, "Can't delete word image from table: %s, %s",
					   errorCode.message().c_str(), std::string(serverErrorCode.server_message()).c_str());
			return errorCode;
		}

		statement = mConnection->prepare_statement(
			"DELETE FROM word WHERE id=?"
		);
		mConnection->execute_statement(statement, std::make_tuple(id), result, errorCode, serverErrorCode);

		if (errorCode) {
			log::error(TAG, "Can't delete word from table: %s, %s",
					   errorCode.message().c_str(), std::string(serverErrorCode.server_message()).c_str());
			return errorCode;
		}

		mConnection->query("SET FOREIGN_KEY_CHECKS = 1", result);
		mConnection->query("COMMIT", result);

		return {};
	}

	auto SyncDictDao::load(const db::row& row) -> boost::system::result<Word, std::string> {
		Word word;

		if (row.at(0).is_int64()) {
			word.id = row.at(0).get_int64();
		} else {
			return "Get field by id=0 is invalid";
		}

		if (row.at(1).is_string()) {
			word.name = row.at(1).get_string();
		} else {
			return "Get field by id=1 is invalid";
		}

		if (row.at(2).is_int64()) {
			word.index = row.at(2).get_int64();
		} else {
			return "Get field by id=2 is invalid";
		}

		if (row.at(3).is_string()) {
			WordType type;
			if (boost::describe::enum_from_string(std::string(row.at(3).get_string()).c_str(), type)) {
				word.type = type;
			} else {
				word.type = WordType::NOUN;
			}
		} else {
			return "Get field by id=3 is invalid";
		}

		if (row.at(4).is_int64()) {
			word.image.id = row.at(4).get_int64();
		} else {
			return "Get field by id=4 is invalid";
		}

		if (row.at(5).is_string()) {
			try {
				word.image.url = boost::urls::parse_uri(row.at(5).get_string()).value();
			} catch (...) {
				word.image.url = boost::urls::parse_uri("http://unknown.org").value();
			}
		} else {
			return "Get field by id=5 is invalid";
		}

		if (row.at(6).is_int64()) {
			word.image.width = static_cast<int32_t>(row.at(6).get_int64());
		} else {
			return "Get field by id=6 is invalid";
		}

		if (row.at(7).is_int64()) {
			word.image.height = static_cast<int32_t>(row.at(7).get_int64());
		} else {
			return "Get field by id=7 is invalid";
		}

		return word;
	}

	auto SyncDictDao::getById(uint64_t id) -> boost::system::result<Word> {
		boost::system::error_code errorCode;
		db::diagnostics serverErrorCode;
		db::results result;

		db::statement statement = mConnection->prepare_statement(R"xxx(
			SELECT word.id AS word_id, word.name, word.`index`, word.type,
				   word_image.id AS word_image_id, word_image.url,
                   word_image.width, word_image.height
			FROM word
			LEFT JOIN word_image ON word.id = word_image.id
			WHERE word.id=?
		)xxx");
		mConnection->execute_statement(statement, std::make_tuple(id), result, errorCode, serverErrorCode);

		if (errorCode) {
			log::error(TAG, "Can't get word by id=%zu from table: %s, %s", id,
					   errorCode.message().c_str(), std::string(serverErrorCode.server_message()).c_str());
			return errorCode;
		} else if (result.rows().empty()) {
			log::error(TAG, "Can't find word with id=%zu", id);
			return db::make_error_code(db::common_server_errc::er_wrong_value_count);
		}

		db::row firstRow = result.rows().at(0);
		boost::system::result<Word, std::string> word = load(firstRow);

		if (word.has_value()) {
			return *word;
		} else {
			log::error(TAG, "Load word error: %s", word.error().c_str());
			return {};
		}
	}

	auto SyncDictDao::getAll() -> boost::system::result<std::vector<Word>> {
		boost::system::error_code errorCode;
		db::diagnostics serverErrorCode;
		db::results result;
		std::vector<Word> words;

		mConnection->query(R"xxx(
			SELECT word.id AS word_id, word.name, word.`index`, word.type,
				   word_image.id AS word_image_id, word_image.url,
				   word_image.width, word_image.height
			FROM word
			LEFT JOIN word_image ON word.id = word_image.id
		)xxx",
		result, errorCode, serverErrorCode);

		if (errorCode) {
			log::error(TAG, "Can't get all words from table: %s, %s",
					   errorCode.message().c_str(), std::string(serverErrorCode.server_message()).c_str());
			return errorCode;
		} else if (result.rows().empty()) {
			log::error(TAG, "Can't find all words");
			return db::make_error_code(db::common_server_errc::er_wrong_value_count);
		}

		const db::rows_view rows = result.rows();
		for (db::row_view row : rows) {
			boost::system::result<Word, std::string> word = load(row);

			if (word.has_value()) {
				words.push_back(*word);
			} else {
				log::error(TAG, "Load words error: %s", word.error().c_str());
			}
		}

		return words;
	}
}

