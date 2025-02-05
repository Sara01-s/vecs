cmake_minimum_required(VERSION 3.18)
project(examples)

add_executable(examples main.cpp)
target_link_libraries(examples PRIVATE vecs)
