cmake_minimum_required(VERSION 3.18)
project(tests)

add_subdirectory(../dependencies/catch2 catch2_build)
add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
