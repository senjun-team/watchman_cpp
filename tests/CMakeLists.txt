cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

find_package(GTest ${LEVEL_FIND_PACKAGE})

macro(add_tests PREFIX TESTS)
    foreach (TEST ${TESTS})
        add_executable("${PREFIX}_${TEST}" ${TEST}.cpp)
        add_test(NAME "${PREFIX}_${TEST}" COMMAND "${PREFIX}_${TEST}")
        target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)
        target_link_libraries("${PREFIX}_${TEST}"
                ${GTEST_BOTH_LIBRARIES}
                ${PROJECT_NAME}_core
                DockerClient)
    endforeach ()
endmacro()

add_compile_options(-Wall -Wextra -pedantic)
add_subdirectory(docker_wrapper)
add_subdirectory(service)
add_subdirectory(parser)
