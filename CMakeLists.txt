cmake_minimum_required(VERSION 3.15.2)
project(Tickets-System-2025)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${PROJECT_SOURCE_DIR}/src/)
include_directories(${PROJECT_SOURCE_DIR}/include/)

set(CMAKE_CXX_FLAGS "-finput-charset=UTF-8 -fexec-charset=UTF-8 -O2")

file(GLOB_RECURSE main_src src/*.cpp)

add_executable(code ${main_src}) 