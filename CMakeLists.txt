cmake_minimum_required(VERSION 3.20.0)

project(coro_fiber
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS off)
set(CMAKE_C_EXTENSIONS off)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -pedantic-errors -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}")

add_executable(coro_fiber_test main.cxx)

target_include_directories(coro_fiber_test PRIVATE "${CMAKE_SOURCE_DIR}/include")
