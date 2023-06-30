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

#include "format/ProtobufParser.hpp"

#include <boost/url/parse.hpp>
#include <fstream>

namespace lynx {

	ProtobufParser::ProtobufParser() {}
	ProtobufParser::~ProtobufParser() {}

	auto ProtobufParser::serializeToBinary(const std::string& fileName, const Word& word) -> boost::system::result<void> {
		pb::RemoteWord remoteWord = convert(word);

		std::ofstream ofs(fileName, std::ios_base::out | std::ios_base::binary);

		if (!ofs.is_open()) {
			return std::make_error_code(std::io_errc::stream);
		}

		if (!remoteWord.SerializeToOstream(&ofs)) {
			return std::make_error_code(std::errc::io_error);
		}

		return {};
	}

	auto ProtobufParser::deserializeFromBinary(const std::string& fileName) -> boost::system::result<Word> {
		pb::RemoteWord remoteWord;
		Word word = {};

		std::ifstream ifs(fileName, std::ios_base::in | std::ios_base::binary);

		if (!ifs.is_open()) {
			return std::make_error_code(std::io_errc::stream);
		}

		if (remoteWord.ParseFromIstream(&ifs)){
			return convert(remoteWord);
		} else {
			return std::make_error_code(std::errc::io_error);
		}
	}

	auto ProtobufParser::serializeToText(const Word& word) -> boost::system::result<std::string> {
		pb::RemoteWord remoteWord = convert(word);
		std::string text;

		if (remoteWord.SerializeToString(&text)) {
			return text;
		} else {
			return std::make_error_code(std::errc::io_error);
		}
	}

	auto ProtobufParser::deserializeFromText(const std::string& text) -> boost::system::result<Word> {
		pb::RemoteWord remoteWord;

		if (remoteWord.ParseFromString(text)) {
			return convert(remoteWord);
		} else {
			return std::make_error_code(std::errc::io_error);
		}
	}

	auto ProtobufParser::serializeToBuffer(const Word& word) -> boost::system::result<std::vector<std::byte>> {
		pb::RemoteWord remoteWord = convert(word);

		const size_t bufferSize = remoteWord.ByteSizeLong();
		std::vector<std::byte> buffer(bufferSize);

		if (remoteWord.SerializeToArray(buffer.data(), static_cast<int32_t>(bufferSize))) {
			return buffer;
		} else {
			return std::make_error_code(std::errc::io_error);
		}
	}

	auto ProtobufParser::deserializeFromBuffer(const std::vector<std::byte>& buffer) -> boost::system::result<Word> {
		pb::RemoteWord remoteWord;

		if (remoteWord.ParseFromArray(buffer.data(), static_cast<int32_t>(buffer.size()))) {
			return convert(remoteWord);
		} else {
			return std::make_error_code(std::errc::io_error);
		}
	}

	auto ProtobufParser::convert(WordType wordType) -> pb::RemoteWordType {
		switch (wordType) {
		case WordType::NOUN:
			return pb::RemoteWordType::NOUN;
		case WordType::ADVERB:
			return pb::RemoteWordType::ADVERB;
		case WordType::ADJECTIVE:
			return pb::RemoteWordType::ADJECTIVE;
		case WordType::VERB:
			return pb::RemoteWordType::VERB;
		default:
			return pb::RemoteWordType::NOUN;
		}
	}

	auto ProtobufParser::convert(const WordImage& wordImage) -> pb::RemoteWordImage {
		pb::RemoteWordImage remoteImage;

		remoteImage.set_id(wordImage.id);
		remoteImage.set_url(wordImage.url.c_str());
		remoteImage.set_width(wordImage.width);
		remoteImage.set_height(wordImage.height);

		return remoteImage;
	}

	auto ProtobufParser::convert(const Word& word) -> pb::RemoteWord {
		pb::RemoteWord remoteWord;

		remoteWord.set_id(word.id);
		remoteWord.set_name(word.name);
		remoteWord.set_index(word.index);
		remoteWord.set_type(convert(word.type));
		*remoteWord.mutable_image() = convert(word.image);

		return remoteWord;
	}

	auto ProtobufParser::convert(pb::RemoteWordType remoteWordType) -> WordType {
		switch (remoteWordType) {
		case pb::RemoteWordType::NOUN:
			return WordType::NOUN;
		case pb::RemoteWordType::ADVERB:
			return WordType::ADVERB;
		case pb::RemoteWordType::ADJECTIVE:
			return WordType::ADJECTIVE;
		case pb::RemoteWordType::VERB:
			return WordType::VERB;
		default:
			return WordType::NOUN;
		}
	}

	auto ProtobufParser::convert(const pb::RemoteWordImage& remoteWordImage) -> WordImage {
		boost::urls::url url;

		try {
			url = boost::urls::parse_uri(remoteWordImage.url()).value();
		} catch (...) {
			url = boost::urls::parse_uri("http://unknown.org").value();
		}

		return WordImage {
			.id = remoteWordImage.id(),
			.url = url,
			.width = remoteWordImage.width(),
			.height = remoteWordImage.height()
		};
	}

	auto ProtobufParser::convert(const pb::RemoteWord& remoteWord) -> Word {
		return Word {
			.id = remoteWord.id(),
			.name = remoteWord.name(),
			.index = remoteWord.index(),
			.type = convert(remoteWord.type()),
			.image = convert(remoteWord.image())
		};
	}
}
