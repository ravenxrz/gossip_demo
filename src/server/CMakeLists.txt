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
target_link_libraries(storage_obj common_obj)

add_library(flags_obj OBJECT server_flags.cc)
target_include_directories(flags_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(service_obj OBJECT service.cc gossip_rpc.cc gossip_task.cc)
target_link_libraries(service_obj gossip_proto storage_obj utils_obj)
target_include_directories(service_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(server main.cc server.cc )
target_link_libraries(server ${DEP_LIBS} service_obj storage_obj flags_obj)

# ##############################################################################
#
# UT define
#
# ##############################################################################

enable_testing()
file(GLOB test_files "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
set(UT_DEP_LIBS gossip_proto protobuf brpc gflags storage_obj service_obj common_obj utils_obj)
foreach(file ${test_files})
  get_filename_component(file_name ${file} NAME)
  if(file_name MATCHES "^test.*\\.cc$")
    get_filename_component(test_target ${file_name} NAME_WE)
    add_executable(${test_target} ${file_name})
    target_link_libraries(${test_target}  gtest_main gtest gmock_main gmock ${UT_DEP_LIBS})
    if(ENABLE_GCOV)
      addcoverage(${test_target})
    endif()
    add_test(NAME ${test_target} COMMAND ${test_target})
  endif()
endforeach()
