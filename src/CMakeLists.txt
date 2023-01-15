cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set(SOURCES
        main.cpp
        common/common.hpp
        server/server.cpp
        server/server.hpp
        core/service.cpp
        core/service.hpp
        server/parser.cpp
        server/parser.hpp
        core/docker_wrapper.cpp
        core/docker_wrapper.hpp)

add_executable(
        ${PROJECT_NAME} ${SOURCES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE RAPIDJSON_HAS_STDSTRING)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/rapidjson/include/)

target_link_libraries(${PROJECT_NAME} DockerClient unifex restinio::restinio)
