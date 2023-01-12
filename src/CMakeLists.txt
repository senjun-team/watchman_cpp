cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set(SOURCES
        main.cpp
        common/common.hpp
        server/server.cpp
        server/server.hpp
        core/service.cpp
        core/service.hpp
        server/parser.cpp server/parser.hpp)

add_executable(
        ${PROJECT_NAME} ${SOURCES}
)

target_link_libraries(${PROJECT_NAME} DockerClient unifex restinio::restinio)
