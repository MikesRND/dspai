cmake_minimum_required(VERSION 3.24)
project(dspai VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add include directory
include_directories(include)

# Tests
enable_testing()

add_executable(component_test tests/component_test.cpp)
add_test(NAME ComponentTest COMMAND component_test)

# Installation
install(DIRECTORY include/dspai DESTINATION include)