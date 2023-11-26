#!/bin/bash

echo
echo "---> Preparing test environment..."
echo

bash tests/scripts/test_cd.sh
bash tests/scripts/test_pwd.sh
bash tests/scripts/test_external_command.sh

