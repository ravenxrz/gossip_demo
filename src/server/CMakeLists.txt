# ##############################################################################
#
# main program define
#
# ##############################################################################

# NOTE: brpc should compile with glog
set(DEP_LIBS
    brpc
    gflags
    protobuf
    leveldb
    gossip_proto
    common_obj
    utils_obj)

add_library(storage_obj OBJECT storage.cc)
target_include_directories(storage_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(storage_obj common_obj utils_obj)

add_library(flags_obj OBJECT server_flags.cc)
target_include_directories(flags_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(service_obj OBJECT service.cc gossip_rpc.cc gossip_task.cc)
target_link_libraries(service_obj gossip_proto storage_obj utils_obj)
target_include_directories(service_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(server-static STATIC server.cc)
target_link_libraries(server-static ${DEP_LIBS} service_obj flags_obj storage_obj)

add_library(server-shared SHARED server.cc)
target_link_libraries(server-shared ${DEP_LIBS} service_obj flags_obj storage_obj)

add_executable(server main.cc)
target_link_libraries(server server-static)
