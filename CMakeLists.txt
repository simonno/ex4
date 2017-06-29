cmake_minimum_required(VERSION 3.6)
project(ex4)

set(CMAKE_C_STANDARD 99)

add_executable(server ex42.c)
add_executable(client ex41.c)
target_link_libraries(server -lpthread)