cmake_minimum_required(VERSION 3.31.6)
project(custom_numbers_lib)

set(CMAKE_C_COMPILER clang)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(OUTPUT_DIR output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT_DIR})

add_compile_options("-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/src/=")
add_compile_options("-g")
add_compile_options("-O0")
make_directory(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_subdirectory(src)
add_subdirectory(examples)
