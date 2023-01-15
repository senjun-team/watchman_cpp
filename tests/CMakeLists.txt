cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

find_package(GTest ${LEVEL_FIND_PACKAGE})

if (NOT GTEST_FOUND)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG release-1.11.0
    )
    FetchContent_MakeAvailable(googletest)
    set(GTEST_BOTH_LIBRARIES gtest gtest_main)
endif ()

macro(add_tests PREFIX TESTS)
    foreach (TEST ${TESTS})
        add_executable("${PREFIX}_${TEST}" ${TEST}.cpp)
        add_test(NAME "${PREFIX}_${TEST}" COMMAND "${PREFIX}_${TEST}")
        target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)
        target_link_libraries("${PREFIX}_${TEST}"
                ${GTEST_BOTH_LIBRARIES}
                ${PROJECT_NAME}
                DockerClient)
    endforeach ()
endmacro()

add_compile_options(-Wall -Wextra -pedantic)
add_subdirectory(docker_wrapper)
