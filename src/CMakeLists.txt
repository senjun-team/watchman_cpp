cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

add_executable(
        ${PROJECT_NAME} main.cpp
        server/server.cpp server/server.hpp watchman/watchman.cpp watchman/watchman.hpp)

target_link_libraries(${PROJECT_NAME} DockerClient unifex restinio::restinio)
