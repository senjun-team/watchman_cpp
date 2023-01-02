cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

add_executable(
        ${PROJECT_NAME} main.cpp
)

target_include_directories(
        ${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/third_party/DockerClient/include/
        ${PROJECT_SOURCE_DIR}/vendor/libunifex/
)

target_link_libraries(${PROJECT_NAME} DockerClient unifex)
