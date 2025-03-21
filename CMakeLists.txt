cmake_minimum_required(VERSION 3.18)
project(vecs LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(vecs)
add_subdirectory(tests)
add_subdirectory(examples)