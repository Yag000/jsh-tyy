#!/bin/bash

VALGRIND="valgrind"
VALGRIND_OPTS="--leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --error-exitcode=1"
TEST="./test"
TMP_DIR="/tmp"
LOG_FILE="$TMP_DIR/valgrind.log"

echo  
echo "====================="
echo
echo "-> Running valgrind..."
echo 


$VALGRIND $VALGRIND_OPTS $TEST 2> $LOG_FILE

if [ $? -eq 0 ]; then
    echo
    echo "Valgrind passed!"
    echo
    exit 0
else
    echo
    echo "Valgrind failed:"
    echo
    cat $LOG_FILE
    echo
    echo "====================="
    exit 1
fi


