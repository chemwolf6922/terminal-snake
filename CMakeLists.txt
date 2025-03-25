cmake_minimum_required(VERSION 3.15)

project(snake LANGUAGES CXX)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Enable verbose build
set(CMAKE_VERBOSE_MAKEFILE ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict warnings and errors
add_compile_options(-Wall -Wextra -Werror -pedantic)

# Create shared library with major version as SO name
add_executable(snake
    main.cpp
    SignalManager.cpp
    Console.cpp
    MainSession.cpp
    GameSession.cpp)

target_link_libraries(snake
    tev-cpp)
