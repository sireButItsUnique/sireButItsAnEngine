cmake_minimum_required(VERSION 3.10)
project(sireButItsAnEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SRCS engine/*.cpp)
add_executable(Engine ${SRCS})
