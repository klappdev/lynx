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

#include <boost/property_tree/ptree.hpp>
#include <boost/system/result.hpp>

#include <vector>

#include "common/Word.hpp"

namespace lynx {

	class XmlParser final {
	public:
		XmlParser() = default;
		~XmlParser() = default;

		auto serializeToText(const Word& word) -> boost::system::result<std::string>;
		auto deserializeFromText(const std::string& text) -> boost::system::result<Word>;

		auto serializeWordsToText(const std::vector<Word>& words) -> boost::system::result<std::string>;
		auto deserializeWordsFromText(const std::string& text) -> boost::system::result<std::vector<Word>>;

		auto serializeToFile(const std::string& fileName, const Word& word) -> boost::system::result<void>;
		auto deserializeFromFile(const std::string& fileName) -> boost::system::result<Word>;

		auto serializeToArchive(const std::string& fileName, const Word& word) -> boost::system::result<void>;
		auto deserializeFromArchive(const std::string& fileName) -> boost::system::result<Word>;

	private:
		void saveToTree(const Word& word);
		auto loadFromTree() -> Word;

		void saveWordsToTree(const std::vector<Word>& word);
		auto loadWordsFromTree() -> std::vector<Word>;

		boost::property_tree::ptree mTree;
	};
}
