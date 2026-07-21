cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(Synthortion VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

add_subdirectory(libs/juce)
add_subdirectory(plugin)
