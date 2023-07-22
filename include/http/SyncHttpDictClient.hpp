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

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include "format/JsonParser.hpp"

namespace beast = boost::beast;
namespace net = boost::asio;
namespace http = beast::http;

namespace lynx {

	class SyncHttpDictClient final {
	public:
		SyncHttpDictClient(const std::string& host, uint16_t port);
		~SyncHttpDictClient();

		[[nodiscard]] bool isStarted() const;

		void start();
		void stop();

		void performPost(const Word& word);
		void performPut(const Word& word);
		void performDelete(uint64_t id);

		[[nodiscard]] auto performGet(uint64_t id) -> Word;
		[[nodiscard]] auto performGet() -> std::vector<Word>;

	private:
		auto prepareRequest(http::verb method, const std::string& target = "", const std::string& body = "")
			-> http::request<http::string_body>;

	private:
		std::string mHost;
		uint16_t mPort;

		net::io_context mContext;
		beast::tcp_stream mStream;

		JsonParser mParser;
		bool mStarted;
	};
}
