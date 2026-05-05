cmake_minimum_required(VERSION 4.0)
project(Karger)

set(CMAKE_CXX_STANDARD 20)

add_executable(Karger ECLgraph.h ECL-CC_11.cpp)
add_executable(Basic basic.cpp ECLgraph.h)
add_executable(Karger-orig ECL-original.cpp ECLgraph.h)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(Karger ${Boost_LIBRARIES})
add_executable(GraphTest GraphTest.cpp GraphTest.h)
