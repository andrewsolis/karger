//
// Created by Andrew Solis on 5/5/2026.
//

//            Copyright Daniel Trebbien 2010.
// Distributed under the Boost Software License, Version 1.0.
//   (See accompanying file LICENSE_1_0.txt or the copy at
//         http://www.boost.org/LICENSE_1_0.txt)

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/exception.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/read_dimacs.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>
#include <boost/graph/property_maps/constant_property_map.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/core/lightweight_test.hpp>

struct edge_t
{
    unsigned long first;
    unsigned long second;
};

/** base boost objects needed for creating lists. adjacency list,
 *  weight map, weight type, and undirected_unweighted graph
 *
 *
 */
typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS,
    boost::no_property, boost::property< boost::edge_weight_t, int > >
    undirected_graph;
typedef boost::property_map< undirected_graph, boost::edge_weight_t >::type
    weight_map_type;
typedef boost::property_traits< weight_map_type >::value_type weight_type;

typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS >
    undirected_unweighted_graph;

static void test_1()
{
    typedef boost::graph_traits<
    undirected_unweighted_graph >::vertex_descriptor vertex_descriptor;
    typedef boost::graph_traits< undirected_unweighted_graph >::edge_descriptor
        edge_descriptor;

    edge_t edges[] = {
        {0, 1}, { 0, 2}, { 0, 3}, { 0, 4},
        { 1, 2}, { 1, 3}, { 2, 3}, { 4, 5},
        { 4, 6}, { 4, 7}, { 5, 6}, { 5, 7},
        { 6, 7}, { 1, 4}, { 3, 6}
    };

    undirected_unweighted_graph g(edges, edges + 14, 8);

    std::map< vertex_descriptor, bool > parity;
    boost::associative_property_map parities(parity);
    std::map< vertex_descriptor, vertex_descriptor > assignment;
    boost::associative_property_map assignments(assignment);

    // calculate min cut
    int w = boost::stoer_wagner_min_cut(g,
    boost::make_constant_property< edge_descriptor >(1),
    boost::vertex_assignment_map(assignments).parity_map(parities));

    std::cout << "The min-cut weight of G is " << w << ".\n" << std::endl;

    size_t i;
    for (i = 0; i < num_vertices(g); ++i)
    {
        if (get(parities, i))
            std::cout << i << std::endl;
    }
    std::cout << std::endl;

    std::cout << "The other set of vertices consists of:" << std::endl;
    for (i = 0; i < num_vertices(g); ++i)
    {
        if (!get(parities, i))
            std::cout << i << std::endl;
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
    if (BOOST_TEST(argc == 1))
    {
    test_1();
    }
    return boost::report_errors();
}
