cmake_minimum_required(VERSION 3.13)

project(SimpleGossip CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_ASAN "enable asn" ON)
option(ENABLE_GCOV "enable gcov" OFF)

# import module
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(AddGCov)

# set global include/link_directories
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

if(ENABLE_ASAN)
  add_compile_options(-fsanitize=address)
  add_link_options(-fsanitize=address)
endif()

if(ENABLE_GCOV)
  add_compile_options(-fprofile-arcs -ftest-coverage)
  add_link_options(-lgcov --coverage)
endif()

add_subdirectory(src)
