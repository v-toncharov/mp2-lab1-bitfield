#!/bin/sh
if [ -z "$CC"  ]; then CC="$(command -v clang)"    && export CC ; fi
if [ -z "$CXX" ]; then CXX="$(command -v clang++)" && export CXX; fi
set -eu
mkdir -p build
ln -s debug/compile_commands.json build/compile_commands.json 2>/dev/null || true
cmake -B build/debug   -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release
