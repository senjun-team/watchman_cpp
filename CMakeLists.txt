cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(watchman-cpp)

set(CMAKE_CXX_STANDARD 20)

find_package(fmt REQUIRED)
find_package(Boost 1.69 REQUIRED COMPONENTS)

add_subdirectory(src)

set(RESTINIO_USE_BOOST_ASIO "shared")
add_subdirectory(third_party/restinio-0.6.17/dev/nodejs/http_parser)
add_subdirectory(third_party/restinio-0.6.17/dev/restinio)

add_subdirectory(third_party/libunifex)
add_subdirectory(third_party/DockerClient)
