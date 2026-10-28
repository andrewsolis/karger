cmake_minimum_required(VERSION 4.0)
project(Karger)

set(CMAKE_CXX_STANDARD 23)

add_executable(Karger ECLgraph.h ECL-CC_11.cpp)
add_executable(Basic basic.cpp ECLgraph.h)
add_executable(Karger-orig ECL-original.cpp ECLgraph.h)

set(BOOST_INCLUDE_LIBRARIES graph)
add_subdirectory(boost EXCLUDE_FROM_ALL)
add_subdirectory(igraph)



add_executable(EGRBoost EGRBoost.h karger_helper.h karger_helper.cpp EGRBoost.cpp)
target_include_directories(EGRBoost PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/graph/test)
target_link_libraries(EGRBoost PRIVATE Boost::graph)

add_executable(EGRigraph EGRigraph.h EGRigraph.cpp)
target_link_libraries(EGRigraph PRIVATE igraph)


add_executable(StoerWagnerExample boost/libs/graph/example/stoer_wagner.cpp)
target_link_libraries(StoerWagnerExample PRIVATE Boost::graph)

add_executable(StoerWagnerBoostTest boost/libs/graph/test/stoer_wagner_test.cpp)
target_link_libraries(StoerWagnerBoostTest PRIVATE Boost::graph)
