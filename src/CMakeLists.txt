cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(CORE_SOURCES
        core/service.cpp
        core/docker_wrapper.cpp
        common/common.cpp
        common/logging.cpp
        common/tar_to_stream.hpp
        )
set(WATCHMAN_CORE "${PROJECT_NAME}")
add_library(${WATCHMAN_CORE} ${CORE_SOURCES})
target_compile_definitions(${WATCHMAN_CORE} PRIVATE RAPIDJSON_HAS_STDSTRING)
target_include_directories(${WATCHMAN_CORE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/rapidjson/include/)
target_include_directories(${WATCHMAN_CORE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/spdlog/include)
target_include_directories(${WATCHMAN_CORE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/fmt/include/)
target_include_directories(${WATCHMAN_CORE} PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(${WATCHMAN_CORE} PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(${WATCHMAN_CORE} PUBLIC DockerClient unifex spdlog fmt::fmt)

#-----------------------------------------------------------------------------------------------------------------------

set(SERVER_SOURCES
        main.cpp
        server/server.cpp
        server/parser.cpp
        )

set(SERVER "${PROJECT_NAME}-executable")
add_executable(${SERVER} ${SERVER_SOURCES})
target_compile_definitions(${SERVER} PRIVATE RAPIDJSON_HAS_STDSTRING)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.json" "${CMAKE_BINARY_DIR}/bin/config.json" COPYONLY)
target_link_libraries(${SERVER} spdlog restinio::restinio fmt::fmt ${WATCHMAN_CORE})
