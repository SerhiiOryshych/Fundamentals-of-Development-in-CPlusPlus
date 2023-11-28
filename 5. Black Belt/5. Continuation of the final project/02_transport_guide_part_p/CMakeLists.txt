cmake_minimum_required(VERSION 3.24)
project(02_transport_guide_part_p)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf 4.24.2 REQUIRED)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalog.proto)
add_executable(02_transport_guide_part_p main.cpp ${PROTO_SRCS} ${PROTO_HDRS} graph.h json.cpp json.h query.h router.h svg.h test_runner.h types.h)
target_link_libraries(02_transport_guide_part_p ${Protobuf_LIBRARIES})
