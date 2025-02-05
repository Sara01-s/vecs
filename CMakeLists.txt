cmake_minimum_required(VERSION 3.18)
project(vecs LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

add_subdirectory(vecs)
add_subdirectory(tests)
add_subdirectory(examples)

include_directories(${CMAKE_SOURCE_DIR}/dependencies/GLWF)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/vulkan)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/vulkan/utility)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/glm)

