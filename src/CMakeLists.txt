cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(CORE_HEADERS
        core/docker_wrapper.hpp
        core/service.hpp
        common/common.hpp
        common/logging.hpp
        common/tar_to_stream.hpp)

set(CORE_SOURCES
        core/service.cpp
        core/docker_wrapper.cpp
        common/common.cpp
        common/logging.cpp
        common/docker_end_answer.cpp
        common/playground.cpp
        common/courses.cpp
        common/answer_common.cpp
        )
set(WATCHMAN_CORE "${PROJECT_NAME}_core")
add_library(${WATCHMAN_CORE} ${CORE_SOURCES})
target_compile_definitions(${WATCHMAN_CORE} PRIVATE RAPIDJSON_HAS_STDSTRING)
target_include_directories(${WATCHMAN_CORE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/rapidjson/include/)
target_include_directories(${WATCHMAN_CORE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/spdlog/include)
target_include_directories(${WATCHMAN_CORE} PUBLIC ${PROJECT_SOURCE_DIR}/third_party/fmt/include/)
target_include_directories(${WATCHMAN_CORE} PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(${WATCHMAN_CORE} PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(${WATCHMAN_CORE} PUBLIC DockerClient spdlog fmt::fmt)

install(TARGETS ${WATCHMAN_CORE} DESTINATION lib)
install(FILES ${CORE_HEADERS} DESTINATION include)

#-----------------------------------------------------------------------------------------------------------------------

set(SERVER_HEADERS
        server/parser.hpp
        server/server.hpp
)

set(SERVER_SOURCES
        main.cpp
        server/server.cpp
        server/parser.cpp
        common/playground.cpp
        common/answer_common.hpp
        common/courses.cpp
        common/answer_common.cpp
)

set(SERVER "${PROJECT_NAME}")
add_executable(${SERVER} ${SERVER_SOURCES})
target_compile_definitions(${SERVER} PRIVATE RAPIDJSON_HAS_STDSTRING)
target_link_libraries(${SERVER} spdlog restinio::restinio fmt::fmt ${WATCHMAN_CORE})

install(TARGETS ${SERVER} DESTINATION bin)
install(FILES ${SERVER_HEADERS} DESTINATION include)
