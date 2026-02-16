cmake_minimum_required(VERSION 3.18)
project(vecs)

add_library(vecs INTERFACE)

if (MSVC)
    target_compile_options(vecs INTERFACE /WX /W4)
else()
    target_compile_options(vecs INTERFACE -Werror -Wall)
endif()

target_include_directories(vecs 
    INTERFACE 
        include
)


