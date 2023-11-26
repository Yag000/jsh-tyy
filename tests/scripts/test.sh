#!/bin/bash

echo "Running test scripts..."

bash tests/scripts/test_cd.sh
bash tests/scripts/test_pwd.sh
bash tests/scripts/test_external_command.sh

echo "Done running test scripts."
