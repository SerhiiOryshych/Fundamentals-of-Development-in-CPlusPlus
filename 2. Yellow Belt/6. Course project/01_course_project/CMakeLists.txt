cmake_minimum_required(VERSION 3.24)
project(01_course_project)

set(CMAKE_CXX_STANDARD 17)

add_executable(01_course_project main.cpp condition_parser_test.cpp condition_parser.cpp condition_parser.h token.cpp token.h test_runner.h database.h date.h node.h date.cpp node.cpp database.cpp)
