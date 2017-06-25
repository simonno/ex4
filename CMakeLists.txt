cmake_minimum_required(VERSION 3.6)
project(ex4)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES ex41.c ex42.c)
add_executable(ex4 ${SOURCE_FILES})