cmake_minimum_required(VERSION 3.24)
project(06_transport_guide_part_t)

set(CMAKE_CXX_STANDARD 14)

find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)

include_directories(06_transport_guide_part_t ${Protobuf_INCLUDE_DIRS})
include_directories(06_transport_guide_part_t ${CMAKE_CURRENT_BINARY_DIR})

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
        proto/name_id_map/db_item_id_name_map.proto
        proto/transport/transport_catalog.proto
        proto/transport/transport_router.proto
        proto/transport/transport_render.proto
        proto/yellow_pages/address.proto
        proto/yellow_pages/company.proto
        proto/yellow_pages/database.proto
        proto/yellow_pages/name.proto
        proto/yellow_pages/phone.proto
        proto/yellow_pages/rubric.proto
        proto/yellow_pages/sphere.proto
        proto/yellow_pages/url.proto
        proto/yellow_pages/working_time.proto
)

add_executable(06_transport_guide_part_t ${PROTO_SRCS} ${PROTO_HDRS} main.cpp graph.h router.h json.cpp json.h data_manager.cpp distance.cpp main.cpp render_manager.cpp render_manager.cpp requests.cpp responses.cpp route_manager.cpp yellow_pages_manager.cpp data_manager.h distance.h interval_map.h ranges.h render_manager.h requests.h responses.h route_manager.h yellow_pages_manager.h svg.cpp xml.cpp db_item_name_id_map.h)

target_link_libraries(06_transport_guide_part_t ${Protobuf_LIBRARIES})
target_link_libraries(06_transport_guide_part_t ${absl_LIBRARIES})
