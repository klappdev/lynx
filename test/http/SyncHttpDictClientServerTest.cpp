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

	TEST(SyncHttpDictClientServerTest, truncateTablesTest)
	{
		SyncDictDao dao(HOST_TEST);
		dao.start();
		dao.truncateTables();
		dao.stop();
	}

	TEST(SyncHttpDictClientServerTest, remoteInsertWordTest)
	{
		SyncHttpDictServer server(HOST_TEST, PORT_TEST);
		std::thread serverThread([](lynx::SyncHttpDictServer& server) {
			server.start();
			EXPECT_TRUE(server.isStarted());
			server.stop();
		}, std::ref(server));

		log::debug(TAG, "Wait while http server is configured");
		std::this_thread::sleep_for(1s);

		SyncHttpDictClient client(HOST_TEST, PORT_TEST);
		std::thread clientThread([](lynx::SyncHttpDictClient& client,
				                    std::thread::native_handle_type serverThreadId) {
			client.start();
			EXPECT_TRUE(client.isStarted());
			client.performPost(WORD_TEST1);
			client.stop();

			boost::system::result<void> result = raiseSignal(serverThreadId, SIGINT);
			if (result.has_error()) {
				log::error(TAG, "Can't send signal to http server thread: %s", result.error().message().c_str());
			}
		}, std::ref(client), serverThread.native_handle());

		clientThread.join();
		serverThread.join();
	}

	TEST(SyncHttpDictClientServerTest, remoteUpdateWordTest)
	{
		SyncHttpDictServer server(HOST_TEST, PORT_TEST);
		std::thread serverThread([](lynx::SyncHttpDictServer& server) {
			server.start();
			EXPECT_TRUE(server.isStarted());
			server.stop();
		}, std::ref(server));

		log::debug(TAG, "Wait while http server is configured");
		std::this_thread::sleep_for(1s);

		SyncHttpDictClient client(HOST_TEST, PORT_TEST);
		std::thread clientThread([](lynx::SyncHttpDictClient& client,
				                    std::thread::native_handle_type serverThreadId) {
			client.start();
			EXPECT_TRUE(client.isStarted());

			auto copyWord = WORD_TEST2;
			copyWord.id = WORD_TEST1.id;
			copyWord.image.id = WORD_TEST1.image.id;
			client.performPut(copyWord);
			client.stop();

			boost::system::result<void> result = raiseSignal(serverThreadId, SIGINT);
			if (result.has_error()) {
				log::error(TAG, "Can't send signal to http server thread: %s", result.error().message().c_str());
			}
		}, std::ref(client), serverThread.native_handle());

		clientThread.join();
		serverThread.join();
	}

	TEST(SyncHttpDictClientServerTest, remoteDeleteWordTest)
	{
		SyncHttpDictServer server(HOST_TEST, PORT_TEST);
		std::thread serverThread([](lynx::SyncHttpDictServer& server) {
			server.start();
			EXPECT_TRUE(server.isStarted());
			server.stop();
		}, std::ref(server));

		log::debug(TAG, "Wait while http server is configured");
		std::this_thread::sleep_for(1s);

		SyncHttpDictClient client(HOST_TEST, PORT_TEST);
		std::thread clientThread([](lynx::SyncHttpDictClient& client,
				                    std::thread::native_handle_type serverThreadId) {
			client.start();
			EXPECT_TRUE(client.isStarted());
			client.performDelete(WORD_TEST1.id);
			client.stop();

			boost::system::result<void> result = raiseSignal(serverThreadId, SIGINT);
			if (result.has_error()) {
				log::error(TAG, "Can't send signal to http server thread: %s", result.error().message().c_str());
			}
		}, std::ref(client), serverThread.native_handle());

		clientThread.join();
		serverThread.join();
	}

	TEST(SyncHttpDictClientServerTest, remoteGetByIdWordTest)
	{
		SyncHttpDictServer server(HOST_TEST, PORT_TEST);
		std::thread serverThread([](lynx::SyncHttpDictServer& server) {
			server.start();
			EXPECT_TRUE(server.isStarted());
			server.stop();
		}, std::ref(server));

		log::debug(TAG, "Wait while http server is configured");
		std::this_thread::sleep_for(1s);

		SyncHttpDictClient client(HOST_TEST, PORT_TEST);
		std::thread clientThread([](lynx::SyncHttpDictClient& client,
				                    std::thread::native_handle_type serverThreadId) {
			client.start();
			EXPECT_TRUE(client.isStarted());
			client.performPost(WORD_TEST1);
			Word remoteWord = client.performGet(WORD_TEST2.id);

			EXPECT_EQ(remoteWord.name, WORD_TEST1.name);
			EXPECT_EQ(remoteWord.index, WORD_TEST1.index);
			EXPECT_EQ(remoteWord.type, WORD_TEST1.type);
			EXPECT_EQ(remoteWord.image.url, WORD_TEST1.image.url);
			EXPECT_EQ(remoteWord.image.width, WORD_TEST1.image.width);
			EXPECT_EQ(remoteWord.image.height, WORD_TEST1.image.height);

			client.stop();

			boost::system::result<void> result = raiseSignal(serverThreadId, SIGINT);
			if (result.has_error()) {
				log::error(TAG, "Can't send signal to http server thread: %s", result.error().message().c_str());
			}
		}, std::ref(client), serverThread.native_handle());

		clientThread.join();
		serverThread.join();
	}

#if 0 /*turn-off*/
	TEST(SyncHttpDictClientServerTest, remoteGetAllWordsTest)
	{
		SyncHttpDictServer server(HOST_TEST, PORT_TEST);
		std::thread serverThread([](lynx::SyncHttpDictServer& server) {
			server.start();
			EXPECT_TRUE(server.isStarted());
			server.stop();
		}, std::ref(server));

		log::debug(TAG, "Wait while http server is configured");
		std::this_thread::sleep_for(1s);

		SyncHttpDictClient client(HOST_TEST, PORT_TEST);
		std::thread clientThread([](lynx::SyncHttpDictClient& client,
				                    std::thread::native_handle_type serverThreadId) {
			const Word WORDS_TEST[] = { WORD_TEST1, WORD_TEST2 };
			client.start();
			EXPECT_TRUE(client.isStarted());

			client.performPost(WORD_TEST2);
			std::vector<Word> words = client.performGet();

			for (size_t i = 0; i < std::size(WORDS_TEST); ++i) {
				EXPECT_EQ(words[i].name, WORDS_TEST[i].name);
				EXPECT_EQ(words[i].index, WORDS_TEST[i].index);
				EXPECT_EQ(words[i].type, WORDS_TEST[i].type);
				EXPECT_EQ(words[i].image.url, WORDS_TEST[i].image.url);
				EXPECT_EQ(words[i].image.width, WORDS_TEST[i].image.width);
				EXPECT_EQ(words[i].image.height, WORDS_TEST[i].image.height);
			}
			client.stop();

			boost::system::result<void> result = raiseSignal(serverThreadId, SIGINT);
			if (result.has_error()) {
				log::error(TAG, "Can't send signal to http server thread: %s", result.error().message().c_str());
			}
		}, std::ref(client), serverThread.native_handle());

		clientThread.join();
		serverThread.join();
	}
#endif

}
