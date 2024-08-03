cmake_minimum_required(VERSION 3.20.0)
project(lynx CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ flags, debug configuration: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "C++ flags, release configuration: ${CMAKE_CXX_FLAGS_RELEASE}")

message("~~~ Conan setup ~~~")
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(Boost REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

set(HEADERS
	include/common/Config.hpp
	include/common/DictCommand.hpp
	include/common/WordType.hpp
	include/common/WordImage.hpp
	include/common/Word.hpp
	
	include/concurrency/ThreadUtils.hpp

	include/db/SyncDictDao.hpp

	include/format/JsonUrlTranslator.hpp
	include/format/JsonParser.hpp
	include/format/ProtobufParser.hpp
	include/format/XmlUrlTranslator.hpp
	include/format/XmlParser.hpp
	
	include/http/SyncHttpDictClient.hpp
	include/http/SyncHttpDictServer.hpp

	include/logging/Logging.hpp
	
	include/net/NetworkUtils.hpp
	include/net/SyncDictClient.hpp
	include/net/SyncDictServer.hpp

	include/rpc/SyncRpcDictClient.hpp
	include/rpc/SyncRpcDictServer.hpp

	include/util/ByteUtils.hpp
	include/util/StringUtils.hpp
)

set(SOURCES
	src/concurrency/ThreadUtils.cpp

	src/db/SyncDictDao.cpp

	src/format/JsonParser.cpp
	src/format/ProtobufParser.cpp
	src/format/XmlParser.cpp

	src/http/SyncHttpDictClient.cpp
	src/http/SyncHttpDictServer.cpp

	src/logging/Logging.cpp
	
	src/net/NetworkUtils.cpp
	src/net/SyncDictClient.cpp
	src/net/SyncDictServer.cpp

	src/rpc/SyncRpcDictClient.cpp
	src/rpc/SyncRpcDictServer.cpp

	src/util/ByteUtils.cpp
	src/util/StringUtils.cpp
)

set(PROTO_SOURCES
    proto/RemoteWord.proto
    proto/RemoteDictService.proto
)

add_library(lynx SHARED
	${HEADERS} ${SOURCES}
	${PROTO_SOURCES}
)

target_include_directories(lynx PUBLIC
	${Boost_INCLUDE_DIRS}
	${protobuf_INCLUDE_DIRS}
	${gRPC_INCLUDE_DIRS}
	${absl_INCLUDE_DIRS}
        ${fmt_INCLUDE_DIRS}
        ${spdlog_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(lynx
	PRIVATE
        fmt::fmt
        spdlog::spdlog
	boost::boost
	Boost::headers
	Threads::Threads
	OpenSSL::Crypto
	OpenSSL::SSL
	protobuf::protobuf
	gRPC::grpc++
)

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET lynx LANGUAGE cpp)
protobuf_generate(TARGET lynx LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

message("~~~ Conan libs: ${CONAN_LIBS}")

enable_testing()
add_subdirectory(test)
