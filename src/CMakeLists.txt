cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(CORE_HEADERS
        core/docker_wrapper.hpp
        core/service.hpp
        common/common.hpp
        common/logging.hpp
        )

set(CORE_SOURCES
        core/service.cpp
        core/docker_wrapper.cpp
        core/parser.cpp
        core/detail/directory_traverser.cpp
        core/detail/container_manipulator.cpp
        common/common.cpp
        common/logging.cpp
        common/project.cpp
        core/code_launcher/detail/process_courses.cpp
        core/code_launcher/detail/process_playground.cpp
        core/code_launcher/detail/process_practice.cpp
        core/code_launcher/detail/code_launchers.cpp
        core/code_launcher/detail/restarting_code_launcher.cpp
        common/config.cpp
        common/detail/answer_common.cpp
        common/detail/containers.cpp
        )
set(WATCHMAN_CORE "${PROJECT_NAME}_core")
add_library(${WATCHMAN_CORE} ${CORE_SOURCES})
target_compile_definitions(${WATCHMAN_CORE} PRIVATE RAPIDJSON_HAS_STDSTRING)
target_include_directories(${WATCHMAN_CORE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/rapidjson/include/)
target_include_directories(${WATCHMAN_CORE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/spdlog/include)
target_include_directories(${WATCHMAN_CORE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/fmt/include/)
target_include_directories(${WATCHMAN_CORE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/libunifex-0.4.0/include)
target_include_directories(${WATCHMAN_CORE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/structarus/include)
target_include_directories(${WATCHMAN_CORE} PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(${WATCHMAN_CORE} PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(${WATCHMAN_CORE} PUBLIC DockerClient spdlog fmt::fmt unifex)

install(TARGETS ${WATCHMAN_CORE} DESTINATION lib)

#-----------------------------------------------------------------------------------------------------------------------

set(SERVER_HEADERS
        server/server.hpp
)

set(SERVER_SOURCES
        main.cpp
        server/server.cpp
        common/detail/answer_common.cpp
)

set(SERVER "${PROJECT_NAME}")
add_executable(${SERVER} ${SERVER_SOURCES})
target_link_libraries(${SERVER} spdlog restinio::restinio fmt::fmt ${WATCHMAN_CORE})

install(TARGETS ${SERVER} DESTINATION bin)
