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

#include <gtest/gtest.h>

#include "db/SyncDictDao.hpp"

#include "logging/Logging.hpp"
#include "common/TestData.hpp"

static constexpr const char* const TAG = "SyncDictDaoTest";
static constexpr const char* const HOST_TEST = "127.0.0.1";

namespace lynx {

	TEST(SyncDictDaoTest, truncateTableTest)
	{
		SyncDictDao dao(HOST_TEST);
		dao.start();
		dao.truncateTables();
		dao.stop();
	}

	TEST(SyncDictDaoTest, tableInsertWordTest)
	{
		SyncDictDao dao(HOST_TEST);
		dao.start();

		dao.insert(WORD_TEST1);
		ASSERT_GT(dao.getLastWordImageId(), 0);
		ASSERT_GT(dao.getLastWordId(), 0);

		dao.stop();
	}

	TEST(SyncDictDaoTest, tableUpdateWordTest)
	{
		SyncDictDao dao(HOST_TEST);
		dao.start();

		auto copyWord = WORD_TEST2;
		copyWord.id = WORD_TEST1.id;
		copyWord.image.id = WORD_TEST1.image.id;
		dao.update(copyWord);

		dao.stop();
	}

	TEST(SyncDictDaoTest, tableDeleteWordTest)
	{
		SyncDictDao dao(HOST_TEST);
		dao.start();
		dao.remove(WORD_TEST1.id);
		dao.stop();
	}

	TEST(SyncDictDaoTest, tableGetByIdWordTest)
	{
		SyncDictDao dao(HOST_TEST);
		dao.start();

		dao.insert(WORD_TEST1);
		boost::system::result<Word> result = dao.getById(dao.getLastWordId());

		if (result.has_error()) {
			log::error(TAG, "Dao get word by id error: %s", result.error().message().c_str());
			EXPECT_TRUE(false);
		}

		EXPECT_EQ(result->name, WORD_TEST1.name);
		EXPECT_EQ(result->index, WORD_TEST1.index);
		EXPECT_EQ(result->type, WORD_TEST1.type);
		EXPECT_EQ(result->image.url, WORD_TEST1.image.url);
		EXPECT_EQ(result->image.width, WORD_TEST1.image.width);
		EXPECT_EQ(result->image.height, WORD_TEST1.image.height);

		dao.stop();
	}

	TEST(SyncDictDaoTest, tableGetAllWordsTest)
	{
		const Word WORDS_TEST[] = { WORD_TEST1, WORD_TEST2 };
		SyncDictDao dao(HOST_TEST);
		dao.start();

		dao.insert(WORD_TEST2);
		boost::system::result<std::vector<Word>> result = dao.getAll();

		if (result.has_error()) {
			log::error(TAG, "Dao get all words error: %s", result.error().message().c_str());
			EXPECT_TRUE(false);
		}

		for (size_t i = 0; i < std::size(WORDS_TEST); ++i) {
			EXPECT_EQ(result.value()[i].name, WORDS_TEST[i].name);
			EXPECT_EQ(result.value()[i].index, WORDS_TEST[i].index);
			EXPECT_EQ(result.value()[i].type, WORDS_TEST[i].type);
			EXPECT_EQ(result.value()[i].image.url, WORDS_TEST[i].image.url);
			EXPECT_EQ(result.value()[i].image.width, WORDS_TEST[i].image.width);
			EXPECT_EQ(result.value()[i].image.height, WORDS_TEST[i].image.height);
		}

		dao.stop();
	}
}
