cmake_minimum_required(VERSION 3.24)
project(01_transport_guide_part_o)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)

include_directories(01_transport_guide_part_o ${Protobuf_INCLUDE_DIRS})
include_directories(01_transport_guide_part_o ${CMAKE_CURRENT_BINARY_DIR})

set(
        absl_LIBRARIES
        absl::algorithm
        absl::base
        absl::debugging
        absl::flat_hash_map
        absl::flags
        absl::memory
        absl::meta
        absl::numeric
        absl::random_random
        absl::strings
        absl::synchronization
        absl::time
        absl::utility
        absl::log
        absl::status
        absl::log_internal_message
        absl::log_internal_check_op
)

protobuf_generate_cpp(
        PROTO_SRCS
        PROTO_HDRS
        transport_catalog.proto
)

add_executable(01_transport_guide_part_o ${PROTO_SRCS} ${PROTO_HDRS} main.cpp graph.h json.cpp)

target_link_libraries(01_transport_guide_part_o ${Protobuf_LIBRARIES})
target_link_libraries(01_transport_guide_part_o ${absl_LIBRARIES})