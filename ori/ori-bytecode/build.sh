#!/bin/bash

# Exit on error
set -e

# Make sure the build dir exists
mkdir -p build

# Build all the c files into the ./build/ori binary
clang -g ./*.c -o ./build/ori -Wall -Wextra -Wpedantic

# Run the binary
./build/ori