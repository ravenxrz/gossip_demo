# ##############################################################################
#
# main program define
#
# ##############################################################################

# NOTE: brpc should compile with glog
set(DEP_LIBS brpc gflags protobuf leveldb gossip_proto common_obj)

add_library(service_obj OBJECT service.cc)
target_link_libraries(service_obj gossip_proto)
target_include_directories(service_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(storage_obj OBJECT storage.cc)
target_include_directories(storage_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(server main.cc)
target_link_libraries(server ${DEP_LIBS} service_obj storage_obj)

# ##############################################################################
#
# UT define
#
# ##############################################################################

enable_testing()
file(GLOB test_files "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
set(UT_DEP_LIBS brpc gflags storage_obj common_obj)
foreach(file ${test_files})
  get_filename_component(file_name ${file} NAME)
  if(file_name MATCHES "^test.*\\.cc$")
    get_filename_component(test_target ${file_name} NAME_WE)
    add_executable(${test_target} ${file_name})
    target_link_libraries(${test_target} gtest_main gtest ${UT_DEP_LIBS})
    if(ENABLE_GCOV)
      addcoverage(${test_target})
    endif()
  endif()
endforeach()
