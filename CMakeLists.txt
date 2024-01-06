cmake_minimum_required(VERSION 3.13)

project(SimpleGossip CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_ASAN "enable asan" OFF)
option(ENABLE_TASN "enable tasn" OFF)
option(ENABLE_GCOV "enable gcov" OFF)
option(BUILD_UT "build ut" ON)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -fPIC -Werror -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -Werror -g -ggdb -fPIC -fno-omit-frame-pointer")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
endif()
message(STATUS "compile flags:" ${CMAKE_CXX_FLAGS})

# import module
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(AddGCov)

# set global include/link_directories
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

if(ENALBE_ASAN AND ENABLE_TASN)
  message(FATAL_ERROR "asan and tasn can't be used at the same time")
endif()

if(ENABLE_ASAN)
  add_compile_options(-fsanitize=address)
  add_link_options(-fsanitize=address)
endif()
if(ENABLE_TASN)
  add_compile_options(-fsanitize=thread)
  add_link_options(-fsanitize=thread)
endif()

if(ENABLE_GCOV)
  add_compile_options(-fprofile-arcs -ftest-coverage)
  add_link_options(-lgcov --coverage)
endif()

add_subdirectory(src)

if(BUILD_UT)
  enable_testing()
  add_subdirectory(test)
endif()
