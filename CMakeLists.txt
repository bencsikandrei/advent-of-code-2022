cmake_minimum_required(VERSION 3.22)

project(aoc-2022)

if(NOT UNIX)
    message(FATAL_ERROR "Not meant to run on Windows!")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-fno-rtti)
add_compile_options(-fno-exceptions)
add_compile_options(-march=native)

add_executable(day1_1 day1/day1_1.cpp)
add_executable(day1_2 day1/day1_2.cpp)

add_executable(day2_1 day2/day2_1.cpp)
add_executable(day2_2 day2/day2_2.cpp)

add_executable(day3_1 day3/day3_1.cpp)
add_executable(day3_2 day3/day3_2.cpp)

add_executable(day4_1 day4/day4_1.cpp)
add_executable(day4_2 day4/day4_2.cpp)
