
file(GLOB files "*.cc")

include_directories(${PROJECT_SOURCE_DIR}/src/utils)

add_library(common_obj OBJECT ${files})
target_include_directories(common_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common_obj)