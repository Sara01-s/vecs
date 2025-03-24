cmake_minimum_required(VERSION 3.18)
project(vecs)

add_library(vecs INTERFACE)

target_compile_options(vecs INTERFACE -Werror -Wall -Wpedantic)

target_include_directories(vecs 
    INTERFACE 
        include
)


