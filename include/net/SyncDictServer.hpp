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

#include "db/SyncDictDao.hpp"
#include "format/XmlParser.hpp"

namespace lynx {

	class SyncDictServer final {
	public:
		SyncDictServer(const std::string& host, uint64_t port);
		~SyncDictServer();

		[[nodiscard]] bool isStarted() const;

		void start();
		void stop();

	private:
		void processMessages();

		void processInsert(const std::string& message);
		void processUpdate(const std::string& message);
		void processDelete(const std::string& message);
		void processGetById(const std::string& message);
		void processGetAll(const std::string& message);

	private:
		boost::asio::io_context mContext;
		boost::asio::ip::tcp::socket mSocket;
		boost::asio::ip::tcp::endpoint mEndpoint;
		boost::asio::ip::tcp::acceptor mAcceptor;

		SyncDictDao dictDao;
		XmlParser mParser;
		bool mStarted;
	};
}
