cmake_minimum_required(VERSION 3.18.4)
project(insound)

set (CMAKE_CXX_STANDARD 20)

option(INSOUND_BUILD_TESTS "Build insound engine unit tests" OFF)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(tools)
