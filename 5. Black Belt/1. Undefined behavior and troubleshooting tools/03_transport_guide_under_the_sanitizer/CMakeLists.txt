cmake_minimum_required(VERSION 3.24)
project(03_transport_guide_under_the_sanitizer)

set(CMAKE_CXX_STANDARD 17)

add_executable(03_transport_guide_under_the_sanitizer main.cpp descriptions.cpp graph.h json.cpp requests.cpp sphere.cpp transport_catalog.cpp transport_router.cpp utils.cpp)
