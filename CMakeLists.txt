project(fb-images)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. DIR_SRCS)
add_executable(fb-images ${DIR_SRCS})

set (FB_VERSION_MAJOR 1)
set (FB_VERSION_MINOR 0)
set (FB_VERSION_PATCH 2)
set (FB_VERSION ${FB_VERSION_MAJOR}.${FB_VERSION_MINOR}.${FB_VERSION_PATCH})
set (PROJECT_VERSION "${FB_VERSION}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

install(TARGETS fb-images DESTINATION bin)
