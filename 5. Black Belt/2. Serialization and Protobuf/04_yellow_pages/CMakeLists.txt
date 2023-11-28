cmake_minimum_required(VERSION 3.24)
project(04_yellow_pages)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)

include_directories(04_yellow_pages ${Protobuf_INCLUDE_DIRS})
include_directories(04_yellow_pages ${CMAKE_CURRENT_BINARY_DIR})

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
        protos/address.proto
        protos/company.proto
        protos/name.proto
        protos/phone.proto
        protos/provider.proto
        protos/signal.proto
        protos/url.proto
        protos/working_time.proto
)

add_executable(04_yellow_pages ${PROTO_SRCS} ${PROTO_HDRS} main.cpp merge.cpp)

target_link_libraries(04_yellow_pages ${Protobuf_LIBRARIES})
target_link_libraries(04_yellow_pages ${absl_LIBRARIES})
