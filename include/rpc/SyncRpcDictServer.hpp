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


#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>

#include "proto/RemoteWord.grpc.pb.h"
#include "proto/RemoteDictService.grpc.pb.h"


#include "db/SyncDictDao.hpp"
#include "format/ProtobufParser.hpp"

namespace lynx {

	class SyncRpcDictServer final : public rpc::RemoteDictService::Service {
	public:
		SyncRpcDictServer(const std::string& host, uint16_t port);
		~SyncRpcDictServer();

		[[nodiscard]] bool isStarted() const;

		void start();
		void stop();

		auto InsertWord(grpc::ServerContext* context, const pb::RemoteWord* request,
						google::protobuf::Empty* response) -> grpc::Status override;
		auto UpdateWord(grpc::ServerContext* context, const pb::RemoteWord* request,
						google::protobuf::Empty* response) -> grpc::Status override;
		auto DeleteWord(grpc::ServerContext* context, const rpc::WordIdRequest* request,
						google::protobuf::Empty* response) -> grpc::Status override;
		auto GetByIdWord(grpc::ServerContext* context, const rpc::WordIdRequest* request,
						pb::RemoteWord* response) -> grpc::Status override;
		auto GetAllWords(grpc::ServerContext* context, const google::protobuf::Empty* request,
						rpc::ListWordsResponse* response) -> grpc::Status override;
		auto Quit(grpc::ServerContext* context, const google::protobuf::Empty* request,
				  google::protobuf::Empty* response) -> grpc::Status override;

		auto getDictDao() const -> const SyncDictDao&;

	private:
		std::string mHost;
		uint16_t mPort;

		std::unique_ptr<grpc::Server> mService;

		SyncDictDao mDictDao;
		ProtobufParser mParser;
		std::atomic_bool mStarted;
	};
}
