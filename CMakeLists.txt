cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(watchman-cpp)

set(CMAKE_CXX_STANDARD 20)

option(SPDLOG_FMT_EXTERNAL "Use external fmt library instead of bundled" ON)

find_package(Boost 1.66 REQUIRED)

add_subdirectory(third_party/fmt)
add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/libunifex)
add_subdirectory(third_party/DockerClient)

set(RESTINIO_USE_BOOST_ASIO "shared")
add_subdirectory(third_party/restinio-0.6.17/dev/nodejs/http_parser)
add_subdirectory(third_party/restinio-0.6.17/dev/restinio)

add_subdirectory(src)

if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif ()
