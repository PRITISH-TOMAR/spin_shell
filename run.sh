#!/bin/bash
set -e

# Clean up old build directory
rm -rf build

# Generate CMake build files using MinGW  DIVE IN DETAIL LATER : : -B build folder , -S :source dir - current
cmake -G "MinGW Makefiles" -B build -S .

# Compile the project
cmake --build build

# Run the shell
exec ./build/shell "$@"
