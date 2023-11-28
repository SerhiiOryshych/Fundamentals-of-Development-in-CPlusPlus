cmake_minimum_required(VERSION 3.14)
project(03_transport_guide_part_q)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)

include_directories(03_transport_guide_part_q ${Protobuf_INCLUDE_DIRS})
include_directories(03_transport_guide_part_q ${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate_cpp(
        PROTO_SRCS
        PROTO_HDRS
        transport_catalog.proto
)

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

add_executable(
        03_transport_guide_part_q ${PROTO_SRCS} ${PROTO_HDRS} main.cpp graph.h json.h json.cpp
        navigation_database.h navigation_database.cpp router.h routes_database.h
        routes_database.cpp stops_database.h stops_database.cpp
        transport_guide_manager.h transport_guide_manager.cpp
        map_database.h map_database.cpp layers.h
        points_computer.h points_computer.cpp stops_database.h stops_builder.cpp
        svg.h svg.cpp route_map.h route_map.cpp
        yellow_pages_database.h request_parser.h)

target_link_libraries(03_transport_guide_part_q ${Protobuf_LIBRARIES})
target_link_libraries(03_transport_guide_part_q ${absl_LIBRARIES})