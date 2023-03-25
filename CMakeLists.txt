cmake_minimum_required(VERSION 3.24)
project(restpp)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

enable_testing()

add_subdirectory(restpp)
add_subdirectory(tests)

