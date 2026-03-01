cmake_minimum_required(VERSION 3.31.6)

project(BFC)

set(CMAKE_C_STANDARD 23)

add_executable(
        bfc

        src/vec.c
        src/vec.h

        src/main.c
)

add_library(
        bfstd

        src/stdlib/stdlib.c
)
