cmake_minimum_required(VERSION 3.30)

project(embedded-prototype)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)


add_subdirectory(generator-1)
add_subdirectory(ST7920)
add_subdirectory(lib)
