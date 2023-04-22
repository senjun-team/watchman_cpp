cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(watchman_cpp)

set(CMAKE_CXX_STANDARD 20)

set(BUILD_SHARED_LIBS FALSE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(SPDLOG_FMT_EXTERNAL "Use external fmt library instead of bundled" ON)

find_package(Boost 1.66 REQUIRED)

add_subdirectory(third_party/fmt)
add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/DockerClient)

set(RESTINIO_USE_BOOST_ASIO "shared")
add_subdirectory(third_party/restinio-0.6.17/dev/nodejs/http_parser)
add_subdirectory(third_party/restinio-0.6.17/dev/restinio)

# TODO make it compile
#add_compile_options(-Werror -Wpedantic)
add_subdirectory(src)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/accomodation/etc/watchman_config.json" "${CMAKE_BINARY_DIR}/bin/watchman_config.json" COPYONLY)

if (BUILD_TESTING)
    enable_testing()
    target_compile_definitions(${PROJECT_NAME} PUBLIC TEST_DATA_DIR= "${CMAKE_CURRENT_SOURCE_DIR}/tests/data/")
    add_subdirectory(tests)
endif ()
