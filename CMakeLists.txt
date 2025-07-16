cmake_minimum_required(VERSION 3.10)
project(sireButItsAnEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-mbmi -mbmi2)

# file(GLOB SRCS engine/*.cpp)
# add_executable(Engine ${SRCS})

add_executable(Engine
    engine/main.cpp
    engine/Board.cpp
    engine/helper.cpp
    engine/MoveGenGen.cpp
    engine/MoveGenInit.cpp
    engine/Move.cpp
    engine/Search.cpp
)