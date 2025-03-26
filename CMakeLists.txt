cmake_minimum_required(VERSION 3.31)
project(ConfProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(ConfProject src/main.cpp
                src/gost.h src/gost.cpp
                src/BS_thread_pool.hpp)

