#!/bin/bash

AUTOTEST_DIR=".sy5-2023-2024-projet-jsh-autotests.nosync"
TESTS_DIR="test-files"
TEST_DIR_NAME="tyy"
TYY_TEST="$PWD/tests/professor-tests"

function help() {
    echo "Usage: $1 test_professor.sh"
    exit 0
}

function create_directory() {
    cp -r $TYY_TEST ./$AUTOTEST_DIR/$TESTS_DIR/$TEST_DIR_NAME
}

function delete_directory() {
    rm -rf ./$AUTOTEST_DIR/$TESTS_DIR/$TEST_DIR_NAME
}

if [ "$#" -ne "1" ]; then 
    help $0
fi

create_directory
./$1
result=$?
delete_directory

exit $result
