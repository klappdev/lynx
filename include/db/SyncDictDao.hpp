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
#include <boost/asio/ssl/context.hpp>
#include <boost/mysql.hpp>

#include "common/Word.hpp"

namespace lynx {

	class SyncDictDao final {
	public:
		SyncDictDao(const std::string& host);
		~SyncDictDao();

		[[nodiscard]] bool isStarted() const;

		void start();
		void stop();

		auto insert(const Word& word) -> boost::system::result<void>;
		auto update(const Word& word) -> boost::system::result<void>;
		auto remove(uint64_t id) -> boost::system::result<void>;

		auto getById(uint64_t id) -> boost::system::result<Word>;
		auto getAll() -> boost::system::result<std::vector<Word>>;

		[[nodiscard]] auto getLastWordId() const -> uint64_t;
		[[nodiscard]] auto getLastWordImageId() const -> uint64_t;

		void truncateTables();

	private:
		void createTables();
		auto load(const boost::mysql::row& row) -> boost::system::result<Word, std::string>;

		std::string mHost;
		boost::asio::io_context mContext;
		std::unique_ptr<boost::mysql::tcp_ssl_connection> mConnection;

		uint64_t mLastWordId;
		uint64_t mLastWordImageId;
		bool mStarted;
	};
}
