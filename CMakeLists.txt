cmake_minimum_required(VERSION 3.20.0)

project(coro-fiber
    VERSION 1.0.0
    LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS off)
set(CMAKE_C_EXTENSIONS off)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")

file(GLOB_RECURSE CXX_SRC "src/*")
add_library(coro_fiber SHARED ${CXX_SRC})
target_include_directories(coro_fiber PRIVATE "include")
