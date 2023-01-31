cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(watchman-cpp)

set(CMAKE_CXX_STANDARD 20)

find_package(fmt REQUIRED)
include_directories(${FMT_INCLUDEDIR}) # set path in cmake options

find_package(Boost 1.66 REQUIRED)

set(RESTINIO_USE_BOOST_ASIO "shared")
add_subdirectory(third_party/restinio-0.6.17/dev/nodejs/http_parser)
add_subdirectory(third_party/restinio-0.6.17/dev/restinio)

add_subdirectory(third_party/libunifex)
add_subdirectory(third_party/DockerClient)
add_subdirectory(third_party/spdlog)

add_subdirectory(src)

if (BUILD_TESTING)
    enable_testing()
    target_compile_definitions(${PROJECT_NAME} PUBLIC TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data/")
    add_subdirectory(tests)
endif ()
