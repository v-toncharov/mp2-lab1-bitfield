cmake_minimum_required(VERSION 4.1)
project(finite-sets)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_COLOR_DIAGNOSTICS 1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(tests)
add_subdirectory(gtest)
