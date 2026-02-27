cmake_minimum_required(VERSION 3.31.6) # Ubuntu, f- u!

project(BFC)

add_executable(
    bfc

    src/main.c
)

add_library(
        stdbf
        src/stdlib.c
)

target_compile_options(bfc PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(stdbf PRIVATE -Wall -Wextra -Wpedantic)