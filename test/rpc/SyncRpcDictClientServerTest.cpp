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

#include "rpc/SyncRpcDictClient.hpp"
#include "rpc/SyncRpcDictServer.hpp"

#include "logging/Logging.hpp"
#include "common/TestData.hpp"
#include "concurrency/ThreadUtils.hpp"

static constexpr const char* const TAG = "SyncRpcDictClientServerTest";
static constexpr const char* const CLIENT_HOST_TEST = "127.0.0.1";
static constexpr const char* const SERVER_HOST_TEST = "0.0.0.0";
static constexpr uint16_t PORT_TEST = 50051;

using namespace std::chrono_literals;

namespace lynx {

	class SyncRpcDictClientServerTest : public testing::Test {
	public:
		SyncRpcDictClientServerTest()
			: mClient(CLIENT_HOST_TEST, PORT_TEST) {

			mServerThread = std::make_unique<std::thread>(std::thread([]() {
				SyncRpcDictServer server(SERVER_HOST_TEST, PORT_TEST);
				server.start();
				EXPECT_TRUE(server.isStarted());
				server.stop();
			}));

			mClient.start();
		}

		~SyncRpcDictClientServerTest() {
			mClient.stop();
			mServerThread->join();
		}

		void remoteInsertWordTest();
		void remoteUpdateWordTest();
		void remoteDeleteWordTest();
		void remoteGetByIdWordTest();
		void remoteGetAllWordsTest();

	protected:
		SyncRpcDictClient mClient;

		std::unique_ptr<std::thread> mServerThread;
	};

	TEST(SyncRpcDictClientServerTest_0, truncateTableTest)
	{
		SyncDictDao dao(CLIENT_HOST_TEST);
		dao.start();
		dao.truncateTables();
		dao.stop();
	}

	TEST_F(SyncRpcDictClientServerTest, runAllTests)
	{
		log::debug(TAG, "Wait while rpc server is configured");
		std::this_thread::sleep_for(1s);

		remoteInsertWordTest();
		remoteUpdateWordTest();
		remoteDeleteWordTest();
		remoteGetByIdWordTest();
		remoteGetAllWordsTest();

		mClient.performQuit();

		boost::system::result<void> result = raiseSignal(mServerThread->native_handle(), SIGINT);
		if (result.has_error()) {
			log::error(TAG, "Can't send signal to rpc server thread: %s", result.error().message().c_str());
		}
	}

	void SyncRpcDictClientServerTest::remoteInsertWordTest() {
		log::debug(TAG, "Wait while rpc server is configured");
		std::this_thread::sleep_for(1s);

		EXPECT_TRUE(mClient.isStarted());

		mClient.performInsert(WORD_TEST1);

	}

	void SyncRpcDictClientServerTest::remoteUpdateWordTest() {
		EXPECT_TRUE(mClient.isStarted());

		auto copyWord = WORD_TEST2;
		copyWord.id = WORD_TEST1.id;
		copyWord.image.id = WORD_TEST1.image.id;
		mClient.performUpdate(copyWord);
	}

	void SyncRpcDictClientServerTest::remoteDeleteWordTest() {
		EXPECT_TRUE(mClient.isStarted());

		mClient.performDelete(WORD_TEST1.id);
	}

	void SyncRpcDictClientServerTest::remoteGetByIdWordTest() {
		EXPECT_TRUE(mClient.isStarted());

		mClient.performInsert(WORD_TEST1);
		Word result = mClient.performGetById(WORD_TEST2.id);

		EXPECT_EQ(result.name, WORD_TEST1.name);
		EXPECT_EQ(result.index, WORD_TEST1.index);
		EXPECT_EQ(result.type, WORD_TEST1.type);
		EXPECT_EQ(result.image.url, WORD_TEST1.image.url);
		EXPECT_EQ(result.image.width, WORD_TEST1.image.width);
		EXPECT_EQ(result.image.height, WORD_TEST1.image.height);
	}

	void SyncRpcDictClientServerTest::remoteGetAllWordsTest() {
		const Word WORDS_TEST[] = { WORD_TEST1, WORD_TEST2 };

		EXPECT_TRUE(mClient.isStarted());

		mClient.performInsert(WORD_TEST2);
		std::vector<Word> result = mClient.performGetAll();

		for (size_t i = 0; i < std::size(WORDS_TEST); ++i) {
			EXPECT_EQ(result[i].name, WORDS_TEST[i].name);
			EXPECT_EQ(result[i].index, WORDS_TEST[i].index);
			EXPECT_EQ(result[i].type, WORDS_TEST[i].type);
			EXPECT_EQ(result[i].image.url, WORDS_TEST[i].image.url);
			EXPECT_EQ(result[i].image.width, WORDS_TEST[i].image.width);
			EXPECT_EQ(result[i].image.height, WORDS_TEST[i].image.height);
		}
	}
}
