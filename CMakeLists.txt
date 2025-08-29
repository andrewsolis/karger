cmake_minimum_required(VERSION 4.0)
project(Karger)

set(CMAKE_CXX_STANDARD 20)

add_executable(Karger karger.cpp ECLgraph.h)
add_executable(Basic basic.cpp ECLgraph.h)
