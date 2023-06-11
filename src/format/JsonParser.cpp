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

#include "format/JsonParser.hpp"
#include "format/JsonUrlTranslator.hpp"

#include <fstream>
#include <boost/pfr.hpp>

namespace lynx {
    JsonParser::JsonParser() {}
    JsonParser::~JsonParser() {}

    auto JsonParser::serializeToText(const Word& word) -> boost::system::result<std::string> {
    	try {
    		return boost::json::serialize(boost::json::value_from(word));
    	} catch (...) {
    		return std::make_error_code(std::errc::not_enough_memory);
    	}
    }

    auto JsonParser::deserializeFromText(const std::string& input) -> boost::system::result<Word> {
    	std::error_code error;

	    boost::json::value value = boost::json::parse(input, error);

	    if (error) return error;

	    return boost::json::value_to<Word>(value);
    }

    auto JsonParser::serializeToFile(const std::string& fileName, const Word& word) -> boost::system::result<void> {
    	std::ofstream ofs(fileName, std::ios_base::out);

    	if (!ofs.is_open()) {
    		return std::make_error_code(std::io_errc::stream);
    	}

    	std::string result = boost::json::serialize(boost::json::value_from(word));
    	ofs << result;

    	return {};
    }

	auto JsonParser::deserializeFromFile(const std::string& fileName) -> boost::system::result<Word> {
		std::error_code error;
		std::ifstream ifs(fileName, std::ios_base::in);

		if (!ifs.is_open()) {
			return std::make_error_code(std::io_errc::stream);
		}

		boost::json::value jsonValue = boost::json::parse(ifs, error);

	    if (error) return error;

	    return boost::json::value_to<Word>(jsonValue);
	}

    void tag_invoke(boost::json::value_from_tag const&, boost::json::value& jsonValue, const WordType& wordType) {
	    jsonValue = static_cast<std::underlying_type_t<WordType>>(wordType);
    }

    auto tag_invoke(boost::json::value_to_tag<WordType> const&, const boost::json::value& jsonValue) -> WordType {
	    return static_cast<WordType>(boost::json::value_to<std::underlying_type_t<WordType>>(jsonValue));
    }

#if LYNX_USE_TEMPLATE_REFLECTION
    void tag_invoke(boost::json::value_from_tag const&, boost::json::value& jsonValue, const WordImage& wordImage) {
	    auto wordImageTuple = boost::pfr::structure_to_tuple(wordImage);

	    jsonValue = boost::json::value_from(wordImageTuple);	
    }

    void tag_invoke(boost::json::value_from_tag const&, boost::json::value& jsonValue, const Word& word) {
	    auto wordTuple = boost::pfr::structure_to_tuple(word);

	    jsonValue = boost::json::value_from(wordTuple);
    }

    auto tag_invoke(boost::json::value_to_tag<WordImage> const&, const boost::json::value& jsonValue) -> WordImage {
	    auto wordImageTuple = boost::json::value_to<std::tuple<boost::urls::url, int32_t, int32_t>>(jsonValue);

	    return std::make_from_tuple<WordImage>(wordImageTuple);
    }
   
    auto tag_invoke(boost::json::value_to_tag<Word> const&, const boost::json::value& jsonValue) -> Word {
	    auto wordTuple = boost::json::value_to<std::tuple<int32_t, std::string, int32_t, WordType, WordImage>>(jsonValue);

	    return std::make_from_tuple<Word>(wordTuple);
    }

#elif LYNX_USE_CUSTOMISE
    void tag_invoke(boost::json::value_from_tag const&, boost::json::value& jsonValue, const WordImage& wordImage) {
	    jsonValue = {
	        { "url", boost::json::value_from(wordImage.url) },
	        { "width", wordImage.width },
	        { "height", wordImage.height }
	    };
    }

    void tag_invoke(boost::json::value_from_tag const&, boost::json::value& jsonValue, const Word& word) {
	    jsonValue = {
	        { "id", word.id },
	        { "name", word.name },
	        { "index", word.index },
	        { "type", boost::json::value_from(word.type) },
	        { "image", boost::json::value_from(word.image) }
	    };
    }

    auto tag_invoke(boost::json::value_to_tag<WordImage> const&, const boost::json::value& jsonValue) -> WordImage {
	    const boost::json::object& object = jsonValue.as_object();

	    return WordImage {
	        boost::json::value_to<boost::urls::url>(object.at("url")),
	        boost::json::value_to<int32_t>(object.at("width")),
	        boost::json::value_to<int32_t>(object.at("height"))
	    };
    }
   
    auto tag_invoke(boost::json::value_to_tag<Word> const&, const boost::json::value& jsonValue) -> Word {
	    const boost::json::object& object = jsonValue.as_object();

	    return Word {
	        boost::json::value_to<int32_t>(object.at("id")),
	        boost::json::value_to<std::string>(object.at("name")),
	        boost::json::value_to<int32_t>(object.at("index")),
	        boost::json::value_to<WordType>(object.at("type")),
	        boost::json::value_to<WordImage>(object.at("image"))
	    };
    }

#endif
}
