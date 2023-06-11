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

#include "format/XmlParser.hpp"
#include "logging/Logging.hpp"
#include "common/TestData.hpp"

static constexpr const char* const TAG = "XmlParserTest";
static constexpr const char* const XML_FILE = "files/remote_word.xml";
static constexpr const char* const XML_ARCHIVE = "files/archive_word.xml";

namespace lynx {

	class XmlParserTest : public testing::Test {
	public:
		XmlParserTest() = default;
		~XmlParserTest() = default;

	protected:
		XmlParser mParser;
	};

	static const char* const WORD_XML_TEST1 = R"xxx(
		<word>
			<id>1</id>
			<name>katze</name>
			<index>1</index>
			<type>1</type>
			<image>
				<url>http://example.org/w/api.php?title=katze</url>
				<width>32</width>
				<height>32</height>
			</image>
		</word>
	)xxx";

	static const char* const WORD_XML_TEST2 = R"xxx(
		<word>
			<id>2</id>
			<name>hunt</name>
			<index>2</index>
			<type>2</type>
			<image>
				<url>http://example.org/w/api.php?title=hunt</url>
				<width>32</width>
				<height>32</height>
			</image>
		</word>
	)xxx";

	TEST_F(XmlParserTest, serializeWordToTextTest)
	{
		boost::system::result<std::string> result = mParser.serializeToText(WORD_TEST1);

		if (result.has_error()) {
			log::error(TAG, "Serialize error: %s", result.error().message().c_str());
			EXPECT_TRUE(false);
		}

		EXPECT_FALSE(result->empty());
		EXPECT_TRUE(result->find("<id>") != std::string::npos);
		EXPECT_TRUE(result->find("<name>") != std::string::npos);
		EXPECT_TRUE(result->find("<index>") != std::string::npos);
		EXPECT_TRUE(result->find("<type>") != std::string::npos);
		EXPECT_TRUE(result->find("<image>") != std::string::npos);
	}

	TEST_F(XmlParserTest, deserializeWordFromTextTest)
	{
		boost::system::result<Word> result = mParser.deserializeFromText(WORD_XML_TEST1);

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

	TEST_F(XmlParserTest, serializeWordsToTextTest)
	{
		boost::system::result<std::string> result = mParser.serializeWordsToText({WORD_TEST1, WORD_TEST2});

		if (result.has_error()) {
			log::error(TAG, "Serialize error: %s", result.error().message().c_str());
			EXPECT_TRUE(false);
		}

		//FIXME: finish in future
	}

	TEST_F(XmlParserTest, deserializeWordsFromTextTest)
	{
		//FIXME: implement in future
	}

	TEST_F(XmlParserTest, serializeWordToFileTest)
	{
		boost::system::result<void> result = mParser.serializeToFile(XML_FILE, WORD_TEST1);

		if (result.has_error()) {
			log::error(TAG, "Serialize error: %s", result.error().message().c_str());
			EXPECT_TRUE(false);
		}

		EXPECT_TRUE(true);
	}

	TEST_F(XmlParserTest, deserializeWordFromFileTest)
	{
		boost::system::result<Word> result = mParser.deserializeFromFile(XML_FILE);

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

	TEST_F(XmlParserTest, serializeWordToArchiveTest)
	{
		boost::system::result<void> result = mParser.serializeToArchive(XML_ARCHIVE, WORD_TEST1);

		if (result.has_error()) {
			log::error(TAG, "Serialize error: %s", result.error().message().c_str());
			EXPECT_TRUE(false);
		}

		EXPECT_TRUE(true);
	}

	TEST_F(XmlParserTest, deserializeWordFromArchiveTest)
	{
		boost::system::result<Word> result = mParser.deserializeFromArchive(XML_ARCHIVE);

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
