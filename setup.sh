#!/bin/bash

BUILD_DIR=cmake-build-debug
CONAN_BIN=/usr/local/bin/conan

set -e
set -x

BASEDIR=$(dirname "$0")
pushd "$BASEDIR"

rm -rf ${BUILD_DIR}

echo "Install Conan missing libraries"
${CONAN_BIN} install . --output-folder=${BUILD_DIR} --build=missing # -s arch=armv7hf

echo "Go into ${BUILD_DIR} directory"
cd ${BUILD_DIR}

echo "Run Cmake build"
cmake .. -G"Eclipse CDT4 - Unix Makefiles" \
         -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
         -DCMAKE_BUILD_TYPE=Release # -DCMAKE_CXX_COMPILER=/opt/gcc-12/bin/g++
cmake --build . -- -j6
