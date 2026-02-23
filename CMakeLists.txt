cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(watchman_cpp)

set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS FALSE)

set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wno-template-body")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(SPDLOG_FMT_EXTERNAL "Use external fmt library instead of bundled" ON)

find_package(Boost 1.66 REQUIRED)
include(GoogleTest)

add_subdirectory(third_party/fmt)
add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/DockerClient)
add_subdirectory(third_party/llhttp-release-v9.1.3)
add_subdirectory(third_party/expected-lite-0.6.3)
add_subdirectory(third_party/libunifex-0.4.0)

set(RESTINIO_ASIO_SOURCE "boost")
add_subdirectory(third_party/restinio-v.0.7.4/dev/restinio)

# TODO make it compile
#add_compile_options(-Werror -Wpedantic)
add_subdirectory(src)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/accomodation/etc/watchman_cpp_config.json" "${CMAKE_BINARY_DIR}/bin/watchman_cpp_config.json" COPYONLY)

if (BUILD_TESTING)
    enable_testing()
    target_compile_definitions(${PROJECT_NAME}_core PUBLIC TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data/")
    add_subdirectory(tests)
endif ()
