#!/bin/bash
set -e

# Clean up old build directory
rm -rf build

# Generate CMake build files using the platform default generator
# Windows (MinGW): pass -G "MinGW Makefiles" manually if needed
# Linux/Mac: default generator (Unix Makefiles / Ninja) is used automatically
cmake -B build -S .

# Compile the project
cmake --build build

# Run the shell
exec ./build/shell "$@"
