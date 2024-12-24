cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

find_package(Catch2 REQUIRED)

# Включаем флаг для возможности `import std`.
# Эта строка должна идти ДО объявления, что проект на C++ (значение CXX).
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

# Для всех целей сборки устанавливаем возможность импорта std в 1.
set(CMAKE_CXX_MODULE_STD 1)

# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Werror -Wall") uncomment for modules
# SET(CMAKE_CXX_STANDARD 23) uncomment for compiling with catch2

project(cpp_experiments LANGUAGES CXX)

add_executable(cpp_experiments)

target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain)

target_sources(cpp_experiments
  PRIVATE
    main.cpp)

target_compile_features(cpp_experiments
  PRIVATE   cxx_std_23
  INTERFACE cxx_std_23)
