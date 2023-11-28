cmake_minimum_required(VERSION 3.24)
project(Mython_2)

set(CMAKE_CXX_STANDARD 17)

add_executable(Mython_2 mython.cpp comparators.cpp object.cpp object_holder.cpp object_holder_test.cpp object_test.cpp parse.cpp parse_test.cpp statement.cpp statement_test.cpp lexer.cpp test_runner.h lexer_test.cpp)
