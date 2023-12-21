#!/bin/bash

AUTOTEST_GIT="https://gaufre.informatique.univ-paris-diderot.fr/geoffroy/sy5-2023-2024-projet-jsh-autotests.git"
AUTOTEST_DIR=".sy5-2023-2024-projet-jsh-autotests.nosync"
TESTS_DIR="test-files"
TEST_DIR_NAME="tyy"
TYY_TEST="$PWD/tests/tests-tyy"

function help() {
    echo "Usage: $1"
    exit 0
}

if [ "$#" -gt "1" ]; then 
    help $0
fi

if [ ! -d "$AUTOTEST_DIR" ]; then
   if ! git clone "$AUTOTEST_GIT" "$AUTOTEST_DIR"; then
      printf "Erreur: je n'ai pas réussi à cloner le dépôt $AUTOTEST_GIT. Abandon.\n" >&2
      exit 1
   fi
fi

mkdir -p ./tmp/PROFESSOR_TESTS_FILES

REF=/ens/geoffroy/Public/sy5/2023-2024/jsh
ON_LULU=false
if [ -x "$REF" ]; then
    ON_LULU=true
fi

mv ./$AUTOTEST_DIR/$TESTS_DIR/* ./tmp/PROFESSOR_TESTS_FILES

if [ $ON_LULU = "true" ]; then
   for f in $(ls $TYY_TEST | grep -v "\-bash-only$"); do
         cp -r $TYY_TEST/$f ./$AUTOTEST_DIR/$TESTS_DIR
   done
else 
   cp -r $TYY_TEST/* ./$AUTOTEST_DIR/$TESTS_DIR/
fi

(
   cd "$AUTOTEST_DIR"
   ./autotests.sh
)

result=$?

rm -rf ./$AUTOTEST_DIR/$TESTS_DIR/*
mv ./tmp/PROFESSOR_TESTS_FILES/* ./$AUTOTEST_DIR/$TESTS_DIR

exit $result
