#!/bin/bash

test_passed=0

echo
echo "-> Running test scripts..."
echo

echo "====================="
echo
echo "--> Running our own tests..."
echo


echo
echo "--> Executing tests..."
echo

./test $@ || test_passed=1

echo 
echo "====================="
echo
if [ $test_passed -eq 0 ]; then
    echo "All tests passed!"
    echo
else
    echo "Some tests failed."
    echo
fi

exit $test_passed
