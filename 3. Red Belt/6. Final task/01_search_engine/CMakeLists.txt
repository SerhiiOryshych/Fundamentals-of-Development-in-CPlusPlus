cmake_minimum_required(VERSION 3.24)
project(01_search_engine)

set(CMAKE_CXX_STANDARD 17)

add_executable(01_search_engine main.cpp search_server.cpp parse.cpp iterator_range.h)
