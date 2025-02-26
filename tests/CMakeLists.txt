cmake_minimum_required(VERSION 3.18)
project(tests)
find_package(Catch2 3 REQUIRED)

add_subdirectory(../dependencies/catch2 catch2_build)
add_executable(tests tests.cpp)

target_include_directories(tests PRIVATE ../vecs/include)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)

enable_testing()
