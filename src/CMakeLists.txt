cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(SOURCES
        main.cpp
        common/common.hpp
        common/common.cpp
        common/logging.hpp
        common/logging.cpp
        server/server.cpp
        server/server.hpp
        core/service.cpp
        core/service.hpp
        server/parser.cpp
        server/parser.hpp
        core/docker_wrapper.cpp
        core/docker_wrapper.hpp)

set(WATCHMAN_EXECUTABLE "${PROJECT_NAME}-executable")
add_executable(${WATCHMAN_EXECUTABLE} ${SOURCES})
target_compile_definitions(${WATCHMAN_EXECUTABLE} PRIVATE RAPIDJSON_HAS_STDSTRING)
target_include_directories(${WATCHMAN_EXECUTABLE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/rapidjson/include/)
target_include_directories(${WATCHMAN_EXECUTABLE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/rapidjson/include/)
target_include_directories(${WATCHMAN_EXECUTABLE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/spdlog/include)
target_include_directories(${WATCHMAN_EXECUTABLE} PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(${WATCHMAN_EXECUTABLE} DockerClient unifex restinio::restinio spdlog)


set(WATCHMAN_LIB "${PROJECT_NAME}")
add_library(${WATCHMAN_LIB} ${SOURCES})
target_compile_definitions(${WATCHMAN_LIB} PRIVATE RAPIDJSON_HAS_STDSTRING)
target_include_directories(${WATCHMAN_LIB} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/rapidjson/include/)
target_include_directories(${WATCHMAN_LIB} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/spdlog/include)
target_include_directories(${WATCHMAN_LIB} PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(${WATCHMAN_LIB} PUBLIC DockerClient unifex restinio::restinio spdlog)
