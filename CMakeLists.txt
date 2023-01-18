cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(watchman-cpp)

set(CMAKE_CXX_STANDARD 20)

find_package(fmt REQUIRED)
find_package(Boost 1.66 REQUIRED)

set(RESTINIO_USE_BOOST_ASIO "shared")
add_subdirectory(third_party/restinio-0.6.17/dev/nodejs/http_parser)
add_subdirectory(third_party/restinio-0.6.17/dev/restinio)

add_subdirectory(third_party/libunifex)
add_subdirectory(third_party/DockerClient)

add_subdirectory(src)

if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif ()
