cmake_minimum_required(VERSION 3.24)
project(03_phone_book)

set(CMAKE_CXX_STANDARD 14)

find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)

include_directories(03_phone_book ${Protobuf_INCLUDE_DIRS})
include_directories(03_phone_book ${CMAKE_CURRENT_BINARY_DIR})

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

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS contact.proto)

add_executable(03_phone_book ${PROTO_SRCS} ${PROTO_HDRS} main.cpp iterator_range.h phone_book.cpp)

target_link_libraries(03_phone_book ${Protobuf_LIBRARIES})
target_link_libraries(03_phone_book ${absl_LIBRARIES})
