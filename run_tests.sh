#!/bin/bash

BUILD_DIR=cmake-build-debug
LYNX_TEST_BIN=${BUILD_DIR}/test/lynx_test

if [[ -e ${LYNX_TEST_BIN} ]]; then
	./${LYNX_TEST_BIN}
else
	echo -e "File ${LYNX_TEST_BIN} doesn't exists!!!"
fi
