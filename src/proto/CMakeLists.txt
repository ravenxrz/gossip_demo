set(SOURCE_FILES)

find_package(Protobuf REQUIRED)
find_program(PROTC protoc REQUIRED)

file(GLOB proto_files ${CMAKE_CURRENT_SOURCE_DIR}/*.proto)
foreach(file "${proto_files}")
  protobuf_generate_cpp(GENERATED_SRC GENERATED_HEADER ${file})
  list(APPEND SOURCE_FILES ${GENERATED_SRC})
endforeach()

add_library(gossip_proto OBJECT ${SOURCE_FILES})
target_include_directories(gossip_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
                                               ${Protobuf_INCLUDE_DIRS})
