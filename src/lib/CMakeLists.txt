include_directories(${CMAKE_SOURCE_DIR}/src/include)

add_library(
    custom_numbers
    STATIC
    generic.c complex.c builtin_types.c
    linked_list.c logger.c custom_numbers.c
)
