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

static constexpr const char* const TAG = "SyncDictDao";
static constexpr const char* const DATABASE_NAME = "dictionary";
static constexpr const char* const TABLE_NAME = "word";
static constexpr const char* const USER_NAME = "user";
static constexpr const char* const PASSWORD = "pass";

namespace lynx {

	SyncDictDao::SyncDictDao(const std::string& host)
		: mHost(host)
        , mStarted(false) {
		log::info(TAG, "Create dict dao");
	}

	SyncDictDao::~SyncDictDao() {
		log::info(TAG, "Destroy dict dao");
	}

	void SyncDictDao::start() {
		log::info(TAG, "Connect to dict table");
	}

	void SyncDictDao::stop() {
		log::info(TAG, "Disconnect from dict table");
	}

	auto SyncDictDao::insert(const Word& word) -> boost::system::result<void> {
		log::info(TAG, "Perform insert \"%s\" word", word.name.c_str());

		mWords.push_back(word);
		return {};
	}

	auto SyncDictDao::update(const Word& word) -> boost::system::result<void> {
		log::info(TAG, "Perform update \"%s\" word", word.name.c_str());

		if (word.id < 1) {
			return std::make_error_code(std::errc::invalid_argument);
		}

		Word& tmp = mWords.at(word.id - 1);
		tmp.name = word.name;
		tmp.index = word.index;
		tmp.type = word.type;
		tmp.image = word.image;

		return {};
	}

	auto SyncDictDao::remove(uint64_t id) -> boost::system::result<void> {
		log::info(TAG, "Perform delete id=%zu", id);

		if (id < 1) {
			return std::make_error_code(std::errc::invalid_argument);
		}

		mWords.erase(mWords.begin() + static_cast<int64_t>(id - 1));

		return {};
	}

	auto SyncDictDao::getById(uint64_t id) -> boost::system::result<Word> {
		log::info(TAG, "Perform get by id=%zu", id);

		if (id < 1) {
			return std::make_error_code(std::errc::invalid_argument);
		}

		auto copyWord = mWords.at(id - 1);

		return copyWord;
	}

	auto SyncDictDao::getAll() -> boost::system::result<std::vector<Word>> {
		log::info(TAG, "Perform get all words");

		auto copyWords = mWords;

		return copyWords;
	}
}

