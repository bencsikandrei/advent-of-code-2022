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
