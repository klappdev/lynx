cmake_minimum_required(VERSION 3.20.0)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)

add_executable(lynx_test
	format/JsonParserTest.cpp
	format/ProtobufParserTest.cpp
	format/XmlParserTest.cpp

	#db/SyncDictDaoTest.cpp
	#net/SyncDictClientServerTest.cpp
	#http/SyncHttpDictClientServerTest.cpp
	rpc/SyncRpcDictClientServerTest.cpp
)

target_include_directories(lynx_test
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(lynx_test
	lynx
	gtest::gtest
        spdlog::spdlog
)

add_test(NAME lynx_test
	COMMAND lynx_test
)
