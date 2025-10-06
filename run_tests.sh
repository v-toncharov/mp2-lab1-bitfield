#!/bin/sh
set -e
if [ ! -d build/debug ]; then ./setup_cmake.sh; fi
cmake --build build/debug
exec build/debug/tests/finitesets-tests
