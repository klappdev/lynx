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

#include "net/SyncDictClient.hpp"
#include "net/SyncDictServer.hpp"

#include "logging/Logging.hpp"
#include "common/TestData.hpp"

static constexpr const char* const TAG = "SyncDictClientServerTest";
static constexpr const char* const HOST_TEST = "127.0.0.1";
static constexpr uint16_t PORT_TEST = 8001;

namespace lynx {

	TEST(SyncDictClientServerTest, remoteInsertWordTest)
	{
		std::thread serverThread([](){
			SyncDictServer server(HOST_TEST, PORT_TEST);

			server.start();
			server.stop();
		});

		std::thread clientThread([](){
			SyncDictClient client(HOST_TEST, PORT_TEST);

			client.start();
			client.performInsert(WORD_TEST1);
			client.performQuit();
			client.stop();
		});

		serverThread.join();
		clientThread.join();
	}

	TEST(SyncDictClientServerTest, remoteUpdateWordTest)
	{
		std::thread serverThread([](){
			SyncDictServer server(HOST_TEST, PORT_TEST);

			server.start();
			server.stop();
		});

		std::thread clientThread([](){
			SyncDictClient client(HOST_TEST, PORT_TEST);

			client.start();
			client.performInsert(WORD_TEST1);

			auto copyWord = WORD_TEST2;
			copyWord.id = WORD_TEST1.id;
			client.performUpdate(copyWord);

			client.performQuit();
			client.stop();
		});

		serverThread.join();
		clientThread.join();
	}

	TEST(SyncDictClientServerTest, remoteDeleteWordTest)
	{
		std::thread serverThread([](){
			SyncDictServer server(HOST_TEST, PORT_TEST);

			server.start();
			server.stop();
		});

		std::thread clientThread([](){
			SyncDictClient client(HOST_TEST, PORT_TEST);

			client.start();
			client.performInsert(WORD_TEST1);
			client.performDelete(WORD_TEST1.id);

			client.performQuit();
			client.stop();
		});

		serverThread.join();
		clientThread.join();
	}

	TEST(SyncDictClientServerTest, remoteGetWordByIdTest)
	{
		std::thread serverThread([](){
			SyncDictServer server(HOST_TEST, PORT_TEST);

			server.start();
			server.stop();
		});

		std::thread clientThread([](){
			SyncDictClient client(HOST_TEST, PORT_TEST);

			client.start();
			client.performInsert(WORD_TEST1);
			Word result = client.performGetById(WORD_TEST1.id);

			EXPECT_EQ(result.id, WORD_TEST1.id);
			EXPECT_EQ(result.name, WORD_TEST1.name);
			EXPECT_EQ(result.index, WORD_TEST1.index);
			EXPECT_EQ(result.type, WORD_TEST1.type);
			EXPECT_EQ(result.image.url, WORD_TEST1.image.url);
			EXPECT_EQ(result.image.width, WORD_TEST1.image.width);
			EXPECT_EQ(result.image.height, WORD_TEST1.image.height);

			client.performQuit();
			client.stop();
		});

		serverThread.join();
		clientThread.join();
	}

	TEST(SyncDictClientServerTest, remoteGetAllWordsTest)
	{
		std::thread serverThread([](){
			SyncDictServer server(HOST_TEST, PORT_TEST);

			server.start();
			server.stop();
		});

		std::thread clientThread([](){
			SyncDictClient client(HOST_TEST, PORT_TEST);
			const Word WORDS_TEST[] = { WORD_TEST1, WORD_TEST2 };

			client.start();
			client.performInsert(WORD_TEST1);
			client.performInsert(WORD_TEST2);
			std::vector<Word> results = client.performGetAll();

			for (size_t i = 0; i < std::size(WORDS_TEST); ++i) {
				EXPECT_EQ(results[i].id, WORDS_TEST[i].id);
				EXPECT_EQ(results[i].name, WORDS_TEST[i].name);
				EXPECT_EQ(results[i].index, WORDS_TEST[i].index);
				EXPECT_EQ(results[i].type, WORDS_TEST[i].type);
				EXPECT_EQ(results[i].image.url, WORDS_TEST[i].image.url);
				EXPECT_EQ(results[i].image.width, WORDS_TEST[i].image.width);
				EXPECT_EQ(results[i].image.height, WORDS_TEST[i].image.height);
			}

			client.performQuit();
			client.stop();
		});

		serverThread.join();
		clientThread.join();
	}
}

