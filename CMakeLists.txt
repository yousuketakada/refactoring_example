cmake_minimum_required(VERSION 3.20)
project(refactoring_a_first_example_cpp)

enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(BuildSettings)

add_subdirectory(src)
