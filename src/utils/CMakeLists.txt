file(GLOB SRCS "*.cc")

add_library(utils_obj OBJECT ${SRCS})
target_include_directories(utils_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(utils_obj pthread)
