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
#include <boost/asio/signal_set.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include "db/SyncDictDao.hpp"
#include "format/JsonParser.hpp"

namespace beast = boost::beast;
namespace net = boost::asio;
namespace http = beast::http;

namespace lynx {

	class SyncHttpDictServer final {
	public:
		SyncHttpDictServer(const std::string& host , uint16_t port);
		~SyncHttpDictServer();

		[[nodiscard]] bool isStarted() const;

		void start();
		void stop();

	private:
		void startSignalHandler();
		void processSession(net::ip::tcp::socket& socket);

		auto handlePostRequest(http::request<http::string_body>&& request) -> std::unique_ptr<http::message_generator>;
		auto handlePutRequest(http::request<http::string_body>&& request) -> std::unique_ptr<http::message_generator>;
		auto handleDeleteRequest(http::request<http::string_body>&& request) -> std::unique_ptr<http::message_generator>;
		auto handleGetByIdRequest(http::request<http::string_body>&& request) -> std::unique_ptr<http::message_generator>;
		auto handleGetAllRequest(http::request<http::string_body>&& request) -> std::unique_ptr<http::message_generator>;

		auto prepareResponse(const std::string& body, http::status status, uint32_t version, bool keepAlive)
			-> http::response<http::string_body>;
		[[nodiscard]] bool checkTarget(boost::core::string_view target) const;

	private:
		std::string mHost;
		uint16_t mPort;

		net::io_context mContext;
		net::ip::tcp::acceptor mAcceptor;
		net::signal_set mSignals;

		SyncDictDao mDictDao;
		JsonParser mParser;
		std::atomic_bool mStarted;
	};
}
