cmake_minimum_required(VERSION 3.13)

project(SimpleGossip CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE INTERNAL "" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set global include/link_directories
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_subdirectory(src)
