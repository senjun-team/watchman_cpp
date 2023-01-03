cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

add_executable(
        ${PROJECT_NAME} main.cpp
)

target_link_libraries(${PROJECT_NAME} DockerClient unifex restinio::restinio)
