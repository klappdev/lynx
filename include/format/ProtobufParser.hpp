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

#include <vector>
#include <boost/system/result.hpp>

#include "common/Word.hpp"
#include "proto/RemoteWord.pb.h"

namespace lynx {

    class ProtobufParser final {
    public:     
	    ProtobufParser();
	    ~ProtobufParser();

	    auto serializeToBinary(const std::string& fileName, const Word& word) -> boost::system::result<void>;
	    auto deserializeFromBinary(const std::string& fileName) -> boost::system::result<Word>;

	    auto serializeToText(const Word& word) -> boost::system::result<std::string>;
	    auto deserializeFromText(const std::string& text) -> boost::system::result<Word>;

	    auto serializeToBuffer(const Word& word) -> boost::system::result<std::vector<std::byte>>;
	    auto deserializeFromBuffer(const std::vector<std::byte>& buffer) -> boost::system::result<Word>;
        
    private:
	    auto convert(WordType wordType) -> pb::RemoteWordType;
	    auto convert(const WordImage& wordImage) -> pb::RemoteWordImage;
	    auto convert(const Word& word) -> pb::RemoteWord;

	    auto convert(pb::RemoteWordType wordType) -> WordType;
	    auto convert(const pb::RemoteWordImage& wordImage) -> WordImage;
	    auto convert(const pb::RemoteWord& word) -> Word;
    };
}
