# ##############################################################################
#
# main program define
#
# ##############################################################################

set(DEP_LIBS
    glog
    brpc
    gflags
    protobuf
    leveldb
    gossip_proto
    common_obj)

add_library(service_obj OBJECT service.cc)
add_library(storage_obj OBJECT storage.cc)
target_include_directories(service_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(storage_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(server main.cc service.cc)
target_link_libraries(server ${DEP_LIBS} service_obj storage_obj)

# ##############################################################################
#
# UT define
#
# ##############################################################################

enable_testing()
file(GLOB test_files "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
set(UT_DEP_LIBS glog gflags storage_obj common_obj)
foreach(file ${test_files})
  get_filename_component(file_name ${file} NAME)
  if(file_name MATCHES "^test.*\\.cc$")
    get_filename_component(test_target ${file_name} NAME_WE)
    add_executable(${test_target} ${file_name})
    target_link_libraries(${test_target} gtest_main gtest ${UT_DEP_LIBS})
  endif()
endforeach()
