
add_executable(client main.cc client.cc)
target_link_libraries(client brpc gflags common_obj gossip_proto protobuf)
