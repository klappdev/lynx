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
#include <thread>

#include "http/SyncHttpDictClient.hpp"
#include "http/SyncHttpDictServer.hpp"

#include "logging/Logging.hpp"
#include "common/TestData.hpp"
#include "concurrency/ThreadUtils.hpp"

static constexpr const char* const TAG = "SyncHttpDictClientServerTest";
static constexpr const char* const HOST_TEST = "127.0.0.1";
static constexpr uint16_t PORT_TEST = 8002;

using namespace std::chrono_literals;

namespace lynx {

	class SyncHttpDictClientServerTest : public testing::Test {
	public:
		SyncHttpDictClientServerTest()
			: mClient(HOST_TEST, PORT_TEST) {

			mServerThread = std::make_unique<std::thread>(std::thread([]() {
				SyncHttpDictServer server(HOST_TEST, PORT_TEST);
				server.start();
				EXPECT_TRUE(server.isStarted());
				server.stop();
			}));

			mClient.start();
		}

		~SyncHttpDictClientServerTest() {
			mClient.stop();
			mServerThread->join();
		}

		void remoteInsertWordTest();
		void remoteUpdateWordTest();
		void remoteDeleteWordTest();
		void remoteGetByIdWordTest();
		void remoteGetAllWordsTest();

	protected:
		SyncHttpDictClient mClient;

		std::unique_ptr<std::thread> mServerThread;
	};

	TEST(SyncHttpDictClientServerTest_0, truncateTablesTest)
	{
		SyncDictDao dao(HOST_TEST);
		dao.start();
		dao.truncateTables();
		dao.stop();
	}

	TEST_F(SyncHttpDictClientServerTest, runAllTests)
	{
		log::debug(TAG, "Wait while http server is configured");
		std::this_thread::sleep_for(1s);

		remoteInsertWordTest();
		remoteUpdateWordTest();
		remoteDeleteWordTest();
		remoteGetByIdWordTest();
		remoteGetAllWordsTest();

		boost::system::result<void> result = raiseSignal(mServerThread->native_handle(), SIGINT);
		if (result.has_error()) {
			log::error(TAG, "Can't send signal to http server thread: %s", result.error().message().c_str());
		}
	}

	void SyncHttpDictClientServerTest::remoteInsertWordTest() {
		log::debug(TAG, "Wait while rpc server is configured");
		std::this_thread::sleep_for(1s);

		EXPECT_TRUE(mClient.isStarted());

		mClient.performPost(WORD_TEST1);
	}

	void SyncHttpDictClientServerTest::remoteUpdateWordTest() {
		EXPECT_TRUE(mClient.isStarted());

		auto copyWord = WORD_TEST2;
		copyWord.id = WORD_TEST1.id;
		copyWord.image.id = WORD_TEST1.image.id;
		mClient.performPut(copyWord);
	}

	void SyncHttpDictClientServerTest::remoteDeleteWordTest() {
		EXPECT_TRUE(mClient.isStarted());

		mClient.performDelete(WORD_TEST1.id);
	}

	void SyncHttpDictClientServerTest::remoteGetByIdWordTest() {
		EXPECT_TRUE(mClient.isStarted());

		mClient.performPut(WORD_TEST1);
		Word result = mClient.performGet(WORD_TEST2.id);

		EXPECT_EQ(result.name, WORD_TEST1.name);
		EXPECT_EQ(result.index, WORD_TEST1.index);
		EXPECT_EQ(result.type, WORD_TEST1.type);
		EXPECT_EQ(result.image.url, WORD_TEST1.image.url);
		EXPECT_EQ(result.image.width, WORD_TEST1.image.width);
		EXPECT_EQ(result.image.height, WORD_TEST1.image.height);
	}


	void SyncHttpDictClientServerTest::remoteGetAllWordsTest() {
#if 0 /*turn-off - implement internal functions*/
		const Word WORDS_TEST[] = { WORD_TEST1, WORD_TEST2 };

		EXPECT_TRUE(mClient.isStarted());

		mClient.performPut(WORD_TEST2);
		std::vector<Word> result = mClient.performGet();

		for (size_t i = 0; i < std::size(WORDS_TEST); ++i) {
			EXPECT_EQ(result[i].name, WORDS_TEST[i].name);
			EXPECT_EQ(result[i].index, WORDS_TEST[i].index);
			EXPECT_EQ(result[i].type, WORDS_TEST[i].type);
			EXPECT_EQ(result[i].image.url, WORDS_TEST[i].image.url);
			EXPECT_EQ(result[i].image.width, WORDS_TEST[i].image.width);
			EXPECT_EQ(result[i].image.height, WORDS_TEST[i].image.height);
		}
#endif
	}
}
