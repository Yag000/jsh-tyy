#!/bin/bash

echo "Creating test environment for external commands..."

if [ -e idonotexist ]; then
	rm -f idonotexit
fi
mkdir -p tmp/iamadirectory

echo "Done creating test environment for external commands."
