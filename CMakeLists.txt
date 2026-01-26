cmake_minimum_required(VERSION 4.0)
project(Karger)

set(CMAKE_CXX_STANDARD 20)

add_executable(Karger ECLgraph.h
        ECL-CC_11.cpp
        graphGen/ECLgraph.h)
add_executable(Basic basic.cpp ECLgraph.h)
add_executable(graphGen graphGen/all_possible_graphs.cpp)
add_executable(Karger-orig ECL-original.cpp ECLgraph.h)
