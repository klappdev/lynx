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

#include <boost/url/parse.hpp>
#include <boost/optional.hpp>

namespace lynx {

	class XmlUrlTranslator final {
	public:
		using internal_type = std::string;
		using external_type = boost::urls::url;

		XmlUrlTranslator() = default;
		~XmlUrlTranslator() = default;

		auto get_value(const internal_type& value) /*override*/ -> boost::optional<external_type> {
			try {
				external_type result = boost::urls::parse_uri(value).value();
				return result;
			} catch (...) {
				return boost::none;
			}
		}

		auto put_value(const external_type& value) /*override*/ -> boost::optional<internal_type> {
			return internal_type(value.c_str());
		}

	private:
		boost::urls::url mUrl;
	};
}

namespace boost::property_tree {

	template<>
	struct translator_between<std::string, boost::urls::url> {
		using type = lynx::XmlUrlTranslator;
	};
}
