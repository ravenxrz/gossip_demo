
file(GLOB files "*.cc")

add_library(common_obj OBJECT ${files})
target_include_directories(common_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})