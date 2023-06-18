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

#include "format/XmlParser.hpp"
#include "format/XmlUrlTranslator.hpp"

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree_serialization.hpp>

#include <iostream> //TMP

static constexpr int32_t ARCHIVE_VERSION = 1;

namespace lynx {

	auto XmlParser::serializeToText(const Word& word) -> boost::system::result<std::string> {
		std::ostringstream stream;

		mWordTree.clear();
		saveToTree(word);

		try {
			boost::property_tree::write_xml(stream, mWordTree);
		} catch (const boost::property_tree::xml_parser_error& e) {
			return std::make_error_code(std::errc::io_error);
		}

		return stream.str();
	}

	auto XmlParser::deserializeFromText(const std::string& text) -> boost::system::result<Word> {
		std::istringstream stream(text);

		mWordTree.clear();

		try {
			boost::property_tree::read_xml(stream, mWordTree);
		} catch (const boost::property_tree::xml_parser_error& e) {
			return std::make_error_code(std::errc::io_error);
		}

		return loadFromTree();
	}

	auto XmlParser::serializeWordsToText(const std::vector<Word>& words) -> boost::system::result<std::string> {
		std::ostringstream stream;

		mWordTree.clear();
		mWordsTree.clear();

		saveWordsToTree(words);

		try {
			boost::property_tree::write_xml(stream, mWordsTree);
		} catch (const boost::property_tree::xml_parser_error& e) {
			return std::make_error_code(std::errc::io_error);
		}

		return stream.str();
	}

	auto XmlParser::deserializeWordsFromText(const std::string& text) -> boost::system::result<std::vector<Word>> {
		std::istringstream stream(text);

		mWordTree.clear();
		mWordsTree.clear();

		try {
			boost::property_tree::read_xml(stream, mWordsTree);
		} catch (const boost::property_tree::xml_parser_error& e) {
			return std::make_error_code(std::errc::io_error);
		}

		return loadWordsFromTree();
	}

	auto XmlParser::serializeToFile(const std::string& fileName, const Word& word) -> boost::system::result<void> {
		mWordTree.clear();

		saveToTree(word);

		try {
			boost::property_tree::write_xml(fileName, mWordTree);
		} catch (const boost::property_tree::xml_parser_error& e) {
			return std::make_error_code(std::errc::io_error);
		}

		return {};
	}

	auto XmlParser::deserializeFromFile(const std::string& fileName) -> boost::system::result<Word> {
		mWordTree.clear();

		try {
			boost::property_tree::read_xml(fileName, mWordTree);
		} catch (const boost::property_tree::xml_parser_error& e) {
			return std::make_error_code(std::errc::io_error);
		}

		return loadFromTree();
	}

	auto XmlParser::serializeToArchive(const std::string& fileName, const Word& word) -> boost::system::result<void> {
		mWordTree.clear();

		std::ofstream ofs(fileName, std::ios_base::out);

		if (!ofs.is_open()) {
			return std::make_error_code(std::io_errc::stream);
		}

		boost::archive::xml_oarchive archive(ofs);

		saveToTree(word);

		boost::property_tree::save(archive, mWordTree, ARCHIVE_VERSION);

		return {};
	}

	auto XmlParser::deserializeFromArchive(const std::string& fileName) -> boost::system::result<Word> {
		mWordTree.clear();

		std::ifstream ifs(fileName, std::ios_base::in);

		if (!ifs.is_open()) {
			return std::make_error_code(std::io_errc::stream);
		}

		boost::archive::xml_iarchive archive(ifs);

		boost::property_tree::load(archive, mWordTree, ARCHIVE_VERSION);

		return loadFromTree();
	}

	void XmlParser::saveToTree(const Word& word) {
		mWordTree.put("word.id", word.id);
		mWordTree.put("word.name", word.name);
		mWordTree.put("word.index", word.index);
		mWordTree.put("word.type", static_cast<std::underlying_type_t<WordType>>(word.type));
		mWordTree.put("word.image.url", word.image.url);
		mWordTree.put("word.image.width", word.image.width);
		mWordTree.put("word.image.height", word.image.height);
	}

	auto XmlParser::loadFromTree() -> Word {
		return Word {
			.id = mWordTree.get<int32_t>("word.id", 0),
			.name = mWordTree.get<std::string>("word.name", "<no_name>"),
			.index = mWordTree.get<int32_t>("word.index", 0),
			.type = static_cast<WordType>(mWordTree.get<std::underlying_type_t<WordType>>("word.type", 1)),
			.image = WordImage {
				.url = mWordTree.get<boost::urls::url>("word.image.url", boost::urls::url("http://unknown.org")),
				.width = mWordTree.get<int32_t>("word.image.width", 0),
				.height = mWordTree.get<int32_t>("word.image.height", 0)
			}
		};
	}

	auto XmlParser::loadFromTree(const boost::property_tree::ptree& tree) -> Word {
		return Word {
			.id = tree.get<int32_t>("id", 0),
			.name = tree.get<std::string>("name", "<no_name>"),
			.index = tree.get<int32_t>("index", 0),
			.type = static_cast<WordType>(tree.get<std::underlying_type_t<WordType>>("type", 1)),
			.image = WordImage {
				.url = tree.get<boost::urls::url>("image.url", boost::urls::url("http://unknown.org")),
				.width = tree.get<int32_t>("image.width", 0),
				.height = tree.get<int32_t>("image.height", 0)
			}
		};
	}

	void XmlParser::saveWordsToTree(const std::vector<Word>& words) {
		for (const Word& word : words) {
			saveToTree(word);
			mWordsTree.add_child("words.word", mWordTree.get_child("word"));
			mWordTree.clear();
		}
	}

	auto XmlParser::loadWordsFromTree() -> std::vector<Word> {
		std::vector<Word> result;

		auto tmpTree = mWordsTree.get_child("words");

		for (const boost::property_tree::ptree::value_type& wordTree : tmpTree) {
			/*boost::property_tree::write_xml(std::cout, wordTree.second);*/
			result.push_back(loadFromTree(wordTree.second));
		}

		return result;
	}
}

