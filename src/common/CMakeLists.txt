
file(GLOB files "*.cc")

add_library(common_obj OBJECT ${files})
target_include_directories(common_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# ##############################################################################
#
# UT define
# TODO(zhanagxingrui): split ut and src
#
# ##############################################################################

add_executable(test_task)
target_link_libraries(test_task common_obj gflags gtest_main gtest brpc)
