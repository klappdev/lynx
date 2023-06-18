cmake_minimum_required(VERSION 3.20.0)
project(lynx CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ flags, debug configuration: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "C++ flags, release configuration: ${CMAKE_CXX_FLAGS_RELEASE}")

message("~~~ Conan setup ~~~")
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
include(${CMAKE_BINARY_DIR}/protobuf-config.cmake)

find_package(Boost REQUIRED)
find_package(Protobuf REQUIRED)

set(HEADERS
	include/common/Config.hpp
	include/common/WordType.hpp
	include/common/WordImage.hpp
	include/common/Word.hpp
	
	include/db/SyncDictDao.hpp

	include/format/JsonUrlTranslator.hpp
	include/format/JsonParser.hpp
	include/format/ProtobufParser.hpp
	include/format/XmlUrlTranslator.hpp
	include/format/XmlParser.hpp
	
	include/logging/Logging.hpp
	
	include/net/DictCommand.hpp
	include/net/NetworkUtils.hpp
	include/net/SyncDictClient.hpp
	include/net/SyncDictServer.hpp

	include/util/ByteUtils.hpp
)

set(SOURCES
	src/db/SyncDictDao.cpp

	src/format/JsonParser.cpp
	src/format/ProtobufParser.cpp
	src/format/XmlParser.cpp

	src/logging/Logging.cpp
	
	src/net/NetworkUtils.cpp
	src/net/SyncDictClient.cpp
	src/net/SyncDictServer.cpp

	src/util/ByteUtils.cpp
)

if (Protobuf_FOUND)
	protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto/RemoteWord.proto)
endif()

add_library(lynx SHARED
	${HEADERS} ${SOURCES}
	${PROTO_HDRS} ${PROTO_SRCS}
)

target_include_directories(lynx PUBLIC
	${Boost_INCLUDE_DIRS}
	${protobuf_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(lynx
	PRIVATE
	boost::boost
	protobuf::protobuf
)


message("~~~ Conan libs: ${CONAN_LIBS}")

enable_testing()
add_subdirectory(test)
