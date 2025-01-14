#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build
cd build/src

# Compile the compiler
./compiler "$(cat ../../input2.txt)" || exit 1
