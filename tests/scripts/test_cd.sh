#!/bin/bash

echo "Creating test environment for cd..."

mkdir -p tmp/dir/subdir
touch tmp/file

# Setup symlink
mkdir -p tmp/dir/symlink_target
ln -s $(pwd)/tmp/dir/symlink_target $(pwd)/tmp/dir/subdir/symlink_source

echo "Done creating test environment for cd."