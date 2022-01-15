cmake_minimum_required(VERSION 3.18)
project(chat)

set(CMAKE_CXX_STANDARD 17)

add_executable(chat main.cpp)
target_link_libraries(chat pthread)