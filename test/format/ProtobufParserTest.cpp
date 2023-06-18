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

#include "format/ProtobufParser.hpp"

#include "logging/Logging.hpp"
#include "common/TestData.hpp"

static constexpr const char* const TAG = "ProtobufParserTest";
static constexpr const char* const PROTO_FILE = "files/remote_word.bin";

namespace lynx {

	class ProtobufParserTest : public testing::Test {
	public:
		ProtobufParserTest() = default;
		~ProtobufParserTest() = default;

	protected:
		static void SetUpTestCase() /*override*/ {
			log::info(TAG, "Test set up");
			mSharedText = new std::string();
			mSharedBuffer = new std::vector<std::byte>();
		}

		static void TearDownTestCase() /*override*/ {
			log::info(TAG, "Test tear down");
			delete mSharedText;
			mSharedText = nullptr;
			delete mSharedBuffer;
			mSharedBuffer = nullptr;
		}

		ProtobufParser mParser;
		static std::string* mSharedText;
		static std::vector<std::byte>* mSharedBuffer;
	};

	std::string* ProtobufParserTest::mSharedText = nullptr;
	std::vector<std::byte>* ProtobufParserTest::mSharedBuffer = nullptr;

	TEST_F(ProtobufParserTest, serializeToBinaryFileTest)
	{
		boost::system::result<void> result = mParser.serializeToBinary(PROTO_FILE, WORD_TEST1);

		if (result.has_error()) {
			log::error(TAG, "Serialize error: %s", result.error().message().c_str());
			EXPECT_TRUE(false);
		}

		EXPECT_TRUE(true);
	}

	TEST_F(ProtobufParserTest, deserializeFromBinaryFileTest)
	{
		boost::system::result<Word> result = mParser.deserializeFromBinary(PROTO_FILE);

		if (result.has_error()) {
			log::error(TAG, "Deserialize error: %s", result.error().message().c_str());
			EXPECT_TRUE(false);
		}

		EXPECT_EQ(result->id, WORD_TEST1.id);
		EXPECT_EQ(result->name, WORD_TEST1.name);
		EXPECT_EQ(result->index, WORD_TEST1.index);
		EXPECT_EQ(result->type, WORD_TEST1.type);
		EXPECT_EQ(result->image.url, WORD_TEST1.image.url);
		EXPECT_EQ(result->image.width, WORD_TEST1.image.width);
		EXPECT_EQ(result->image.height, WORD_TEST1.image.height);

	}

	TEST_F(ProtobufParserTest, serializeToTextTest)
	{
		boost::system::result<std::string> result = mParser.serializeToText(WORD_TEST1);

		if (result.has_error()) {
			log::error(TAG, "Serialize error: %s", result.error().message().c_str());
			EXPECT_TRUE(false);
		}

		*mSharedText = result.value();

		EXPECT_TRUE(result.has_value());
		EXPECT_FALSE(result->empty());
	}

	TEST_F(ProtobufParserTest, deserializeFromTextTest)
	{
		boost::system::result<Word> result = mParser.deserializeFromText(*mSharedText);

		if (result.has_error()) {
			log::error(TAG, "Deserialize error: %s", result.error().message().c_str());
			EXPECT_TRUE(false);
		}

		EXPECT_EQ(result->id, WORD_TEST1.id);
		EXPECT_EQ(result->name, WORD_TEST1.name);
		EXPECT_EQ(result->index, WORD_TEST1.index);
		EXPECT_EQ(result->type, WORD_TEST1.type);
		EXPECT_EQ(result->image.url, WORD_TEST1.image.url);
		EXPECT_EQ(result->image.width, WORD_TEST1.image.width);
		EXPECT_EQ(result->image.height, WORD_TEST1.image.height);
	}

	TEST_F(ProtobufParserTest, serializeToBufferTest)
	{
		boost::system::result<std::vector<std::byte>> result = mParser.serializeToBuffer(WORD_TEST1);

		if (result.has_error()) {
			log::error(TAG, "Serialize error: %s", result.error().message().c_str());
			EXPECT_TRUE(false);
		}

		*mSharedBuffer = result.value();

		EXPECT_TRUE(result.has_value());
		EXPECT_FALSE(result->empty());
	}

	TEST_F(ProtobufParserTest, deserializeFromBufferTest)
	{
		boost::system::result<Word> result = mParser.deserializeFromBuffer(*mSharedBuffer);

		if (result.has_error()) {
			log::error(TAG, "Deserialize error: %s", result.error().message().c_str());
			EXPECT_TRUE(false);
		}

		EXPECT_EQ(result->id, WORD_TEST1.id);
		EXPECT_EQ(result->name, WORD_TEST1.name);
		EXPECT_EQ(result->index, WORD_TEST1.index);
		EXPECT_EQ(result->type, WORD_TEST1.type);
		EXPECT_EQ(result->image.url, WORD_TEST1.image.url);
		EXPECT_EQ(result->image.width, WORD_TEST1.image.width);
		EXPECT_EQ(result->image.height, WORD_TEST1.image.height);
	}
}
