cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(SOURCES
        main.cpp
        common/common.cpp
        common/logging.cpp
        server/server.cpp
        core/service.cpp
        server/parser.cpp
        core/docker_wrapper.cpp
        )

set(WATCHMAN_EXECUTABLE "${PROJECT_NAME}-executable")
add_executable(${WATCHMAN_EXECUTABLE} ${SOURCES})
target_compile_definitions(${WATCHMAN_EXECUTABLE} PRIVATE RAPIDJSON_HAS_STDSTRING)
target_include_directories(${WATCHMAN_EXECUTABLE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/rapidjson/include/)
target_include_directories(${WATCHMAN_EXECUTABLE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/rapidjson/include/)
target_include_directories(${WATCHMAN_EXECUTABLE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/spdlog/include)
target_include_directories(${WATCHMAN_EXECUTABLE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/fmt/include)
target_include_directories(${WATCHMAN_EXECUTABLE} PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(${WATCHMAN_EXECUTABLE} spdlog DockerClient unifex restinio::restinio fmt::fmt)


set(WATCHMAN_LIB "${PROJECT_NAME}")
add_library(${WATCHMAN_LIB} ${SOURCES})
target_compile_definitions(${WATCHMAN_LIB} PRIVATE RAPIDJSON_HAS_STDSTRING)
target_include_directories(${WATCHMAN_LIB} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/rapidjson/include/)
target_include_directories(${WATCHMAN_LIB} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/spdlog/include)
target_include_directories(${WATCHMAN_LIB} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/fmt/include/)
target_include_directories(${WATCHMAN_LIB} PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(${WATCHMAN_LIB} PUBLIC DockerClient unifex restinio::restinio spdlog fmt::fmt)
