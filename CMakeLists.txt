cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(watchman-cpp)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)
add_subdirectory(third_party/DockerClient)
