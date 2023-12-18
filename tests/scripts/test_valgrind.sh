#!/bin/bash

VALGRIND="valgrind"
VALGRIND_OPTS="--leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --error-exitcode=1 -s"
TEST="./test"
TMP_DIR="./tmp"
LOG_FILE="$TMP_DIR/valgrind.log"

echo  
echo "====================="
echo
echo "-> Running valgrind..."
echo 

getopts "f" opt;
case "${opt}" in
    f)
        TRACK_FDS=true
	VALGRIND_OPTS="$VALGRIND_OPTS --track-fds=yes"
	;;
esac
shift 1


$VALGRIND $VALGRIND_OPTS $TEST ${@:2} 2> $LOG_FILE
valgrind_result=$?


if [ "$TRACK_FDS" = true ] && [ $(grep "Open file descriptor" $LOG_FILE | wc -l) -gt 0 ]; then
    valgrind_result=1
fi

if [ $valgrind_result -eq 0 ]; then
    echo
    echo "Valgrind passed!"
    echo
    echo "====================="
    echo
    exit 0
else
    echo
    echo "Valgrind failed:"
    echo
    cat $LOG_FILE
    echo
    echo "====================="
    echo
    exit 1
fi


