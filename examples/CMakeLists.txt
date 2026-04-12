include_directories(${CMAKE_SOURCE_DIR}/src/include)

add_executable(two_integers two_integers.c)
target_link_libraries(two_integers custom_numbers)

add_executable(two_complex_numbers two_complex_numbers.c)
target_link_libraries(two_complex_numbers custom_numbers)

add_executable(two_bignums two_bignums.c)
target_link_libraries(two_bignums custom_numbers)

add_executable(two_rationals two_rationals.c)
target_link_libraries(two_rationals custom_numbers)
