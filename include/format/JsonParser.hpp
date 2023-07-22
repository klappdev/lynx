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

#pragma once

#include <boost/json.hpp>
#include <boost/system/result.hpp>

#include <vector>

#include "common/Config.hpp"
#include "common/Word.hpp"

namespace lynx {

    class JsonParser final {
    public:
	    JsonParser();
	    ~JsonParser();

	    auto serializeToText(const Word& word) -> boost::system::result<std::string>;
	    auto deserializeFromText(const std::string& input) -> boost::system::result<Word>;

	    auto serializeWordsToText(const std::vector<Word>& words) -> boost::system::result<std::string>;
	    auto deserializeWordsFromText(const std::string& input) -> boost::system::result<std::vector<Word>>;

	    auto serializeToFile(const std::string& fileName, const Word& word) -> boost::system::result<void>;
		auto deserializeFromFile(const std::string& fileName) -> boost::system::result<Word>;
    };

    void tag_invoke(boost::json::value_from_tag const&, boost::json::value& jsonValue, const WordType& wordType);
    auto tag_invoke(boost::json::value_to_tag<WordType> const&, const boost::json::value& jsonValue) -> WordType;

#if LYNX_USE_TEMPLATE_REFLECTION || LYNX_USE_CUSTOMISE
    void tag_invoke(boost::json::value_from_tag const&, boost::json::value& jsonValue, const WordImage& wordImage);
    void tag_invoke(boost::json::value_from_tag const&, boost::json::value& jsonValue, const Word& word);

    auto tag_invoke(boost::json::value_to_tag<WordImage> const&, const boost::json::value& jsonValue) -> WordImage;
    auto tag_invoke(boost::json::value_to_tag<Word> const&, const boost::json::value& jsonValue) -> Word;

#elif LYNX_USE_MACRO_REFLECTION
    template<class T,
		class D1 = boost::describe::describe_members<T,
				boost::describe::mod_public | boost::describe::mod_protected>,
		class D2 = boost::describe::describe_members<T, boost::describe::mod_private>,
		class En = std::enable_if_t<boost::mp11::mp_empty<D2>::value && !std::is_union<T>::value>
    >	
    inline void tag_invoke(boost::json::value_from_tag const&, boost::json::value& jsonValue, const T& t);

    template<class T,
		class D1 = boost::describe::describe_members<T,
				boost::describe::mod_public | boost::describe::mod_protected>,
		class D2 = boost::describe::describe_members<T, boost::describe::mod_private>,
		class En = std::enable_if_t<boost::mp11::mp_empty<D2>::value && !std::is_union<T>::value>
    >
    inline auto tag_invoke(boost::json::value_to_tag<T> const&, const boost::json::value& jsonValue);

	#include "JsonParser.ipp"
#endif
}
