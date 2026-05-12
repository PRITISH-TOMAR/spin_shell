#!/bin/bash
set -e

# Clean up old build directory
rm -rf build

# Generate CMake build files using MinGW
cmake -G "MinGW Makefiles" -B build -S .

# Compile the project
cmake --build build

# Run the shell
exec ./build/shell "$@"
