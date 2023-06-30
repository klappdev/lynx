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

#include "common/Word.hpp"

namespace lynx {

   static const char* const URL_TEST = "http://example.org/w/api.php?title=";

   static const Word WORD_TEST1 = {
        .id = 1,
        .name = "katze",
        .index = 1,
        .type = WordType::NOUN,
        .image = {
			.id = 1,
            .url = boost::urls::url(URL_TEST + std::string("katze")),
            .width = 32,
            .height = 32	
        }
   };

   static const Word WORD_TEST2 = {
	   .id = 2,
	   .name = "hunt",
	   .index = 2,
	   .type = WordType::ADJECTIVE,
	   .image = {
		   .id = 2,
		   .url = boost::urls::url(URL_TEST + std::string("hunt")),
		   .width = 48,
		   .height = 48
	   }
  };
}
