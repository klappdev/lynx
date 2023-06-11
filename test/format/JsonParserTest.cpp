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

#include "format/JsonParser.hpp"
#include "logging/Logging.hpp"
#include "common/TestData.hpp"

static constexpr const char* const TAG = "ProtobufParserTest";
static constexpr const char* const JSON_FILE = "files/remote_word.json";

namespace lynx {

    class JsonParserTest : public testing::Test {
    public:
        JsonParserTest() = default;
	    ~JsonParserTest() override = default;	

    protected:
	    JsonParser mParser;
    };

    static const char* const WORD_JSON_TEST =
#if LYNX_USE_CUSTOMISE || LYNX_USE_MACRO_REFLECTION
	R"xxx(
 	    {
 	         "id": 1, 
 	         "name": "katze",
 	         "index": 1,
 	         "type": 1,
 	         "image": {
 		         "url": "http://example.org/w/api.php?title=katze",
 	             "width": 32,
 	             "height": 32
 	         }
 	    }
    )xxx";
#elif LYNX_USE_TEMPLATE_REFLECTION
    R"xxx(
 	    [
 	         1, "katze", 1, 1, 
             ["http://example.org/w/api.php?title=katze", 32, 32]
 	    ]
    )xxx";
#endif

    TEST_F(JsonParserTest, serializeToTextTest)
    {
	    boost::system::result<std::string> result = mParser.serializeToText(WORD_TEST1);

	    if (result.has_error()) {
			log::error(TAG, "Serialize error: %s", result.error().message().c_str());
			EXPECT_TRUE(false);
		}

	    log::debug(TAG, "serialize text: %s", result.value().c_str());
	    EXPECT_FALSE(result->empty());
#if LYNX_USE_CUSTOMISE || LYNX_USE_MACRO_REFLECTION
	    EXPECT_TRUE(result->find("\"id\"") != std::string::npos);
	    EXPECT_TRUE(result->find("\"name\"") != std::string::npos);
	    EXPECT_TRUE(result->find("\"index\"") != std::string::npos);
	    EXPECT_TRUE(result->find("\"type\"") != std::string::npos);
	    EXPECT_TRUE(result->find("\"image\"") != std::string::npos);
#endif
    }

    TEST_F(JsonParserTest, deserializeFromTextTest)
	{
    	boost::system::result<Word> result = mParser.deserializeFromText(WORD_JSON_TEST);

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

    TEST_F(JsonParserTest, serializeToFileTest)
	{
		boost::system::result<void> result = mParser.serializeToFile(JSON_FILE, WORD_TEST1);

		if (result.has_error()) {
			log::error(TAG, "Serialize error: %s", result.error().message().c_str());
			EXPECT_TRUE(false);
		}

		EXPECT_TRUE(true);
	}

    TEST_F(JsonParserTest, deserializeFromFileTest)
	{
		boost::system::result<Word> result = mParser.deserializeFromFile(JSON_FILE);

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
