cmake_minimum_required(VERSION 3.24)
project(05_transport_guide_part_s)

set(CMAKE_CXX_STANDARD 14)

find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)

include_directories(05_transport_guide_part_s ${Protobuf_INCLUDE_DIRS})
include_directories(05_transport_guide_part_s ${CMAKE_CURRENT_BINARY_DIR})

set(absl_LIBRARIES
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
        transport_catalog_proto/address.proto
        transport_catalog_proto/company.proto
        transport_catalog_proto/database.proto
        transport_catalog_proto/descriptions.proto
        transport_catalog_proto/graph.proto
        transport_catalog_proto/map_renderer.proto
        transport_catalog_proto/name.proto
        transport_catalog_proto/phone.proto
        transport_catalog_proto/rubric.proto
        transport_catalog_proto/sphere.proto
        transport_catalog_proto/svg.proto
        transport_catalog_proto/transport_catalog.proto
        transport_catalog_proto/transport_router.proto
        transport_catalog_proto/url.proto
        transport_catalog_proto/working_time.proto
)

add_executable(05_transport_guide_part_s ${PROTO_SRCS} ${PROTO_HDRS} main.cpp algorithm.cpp companies_index.cpp company.cpp company_query.cpp descriptions.cpp graph.cpp hash_combine.cpp json.cpp map.cpp map_renderer.cpp numeric.cpp range.cpp requests.cpp router.cpp rubric.cpp sphere.cpp sphere_serialize.cpp svg.cpp svg_serialize.cpp transport_catalog.cpp transport_router.cpp)

target_link_libraries(05_transport_guide_part_s ${Protobuf_LIBRARIES})
target_link_libraries(05_transport_guide_part_s ${absl_LIBRARIES})
