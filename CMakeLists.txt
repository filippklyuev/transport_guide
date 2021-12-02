cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto)

set(CATALOGUE_FILES main.cpp domain.cpp domain.h geo.cpp geo.h graph.h json.cpp json.h json_builder.cpp json_builder.h json_reader.cpp json_reader.h map_renderer.cpp map_renderer.h ranges.h request_handler.cpp request_handler.h router.h serialization.cpp serialization.h svg.cpp svg.h transport_catalogue.cpp transport_catalogue.h transport_router.cpp transport_router.h transport_catalogue.proto)

add_executable(transport_catalogue ${PROTO_SRC} ${PROTO_HDRS} ${CATALOGUE_FILES})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

#string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
#string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

#target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)

target_link_libraries(transport_catalogue Threads::Threads ${Protobuf_LIBRARY} )