cmake_minimum_required(VERSION 3.24)
project(07_web_server_statistics)

set(CMAKE_CXX_STANDARD 17)

add_executable(07_web_server_statistics main.cpp http_request.h stats.cpp)
