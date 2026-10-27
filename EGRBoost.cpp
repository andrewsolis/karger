//
// Created by Andrew Solis on 5/5/2026.
//

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <print>

#include <boost/graph/adjacency_list.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>

#include "mas_sw_maxflow_oracle.hpp"
#include "karger_helper.h"
#include "ECLgraph.h"


using mas_sw_oracle::weight_type;

using undirected_unweighted_graph = boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS >;

std::string egr_path;

struct edge_t
{
    unsigned long first;
    unsigned long second;
};


//            Copyright Daniel Trebbien 2010.
// Distributed under the Boost Software License, Version 1.0.
//   (See accompanying file LICENSE_1_0.txt or the copy at
//         http://www.boost.org/LICENSE_1_0.txt)
/*
 * Basic test of unweighted graph with a constant weight map
 * (copied from stoer_wagner_test.cpp test4() )
 */
void basicTest() {
    using vd = boost::graph_traits< undirected_unweighted_graph >::vertex_descriptor;
    using ed = boost::graph_traits< undirected_unweighted_graph >::edge_descriptor;

    edge_t edges[] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 0, 4 }, { 1, 4 },
        { 1, 5 }, { 2, 6 }, { 3, 6 }, { 3, 7 }, { 4, 5 }, { 5, 6 }, { 6, 7 },
        { 0, 4 }, { 6, 7 } };
    undirected_unweighted_graph g(edges, edges + 14, 8);

    std::map< vd, bool > parity;
    boost::associative_property_map< std::map< vd, bool > > parities(parity);
    const int w = boost::graph::stoer_wagner_min_cut(g,
        boost::make_constant_property< ed >(weight_type(1)), parities);
    BOOST_TEST_EQ(w, 2);
    const bool parity0 = get(parities, 0);
    BOOST_TEST_EQ(parity0, get(parities, 1));
    BOOST_TEST_EQ(parity0, get(parities, 4));
    BOOST_TEST_EQ(parity0, get(parities, 5));
    const bool parity2 = get(parities, 2);
    BOOST_TEST_NE(parity0, parity2);
    BOOST_TEST_EQ(parity2, get(parities, 3));
    BOOST_TEST_EQ(parity2, get(parities, 6));
    BOOST_TEST_EQ(parity2, get(parities, 7));


    std::println("The min-cut weight of G is {}", w);


}

int main(int argc, char* argv[])
{
    /*
     * 
     */
    if (BOOST_TEST(argc == 2))
    {
        egr_path = argv[1];

        std::println("EGR file path: {}", egr_path);

        basicTest();

        ECLgraph g = readECLgraph(argv[1]);

        std::println("input graph: {} nodes and {} edges(duplicates x 2)", g.nodes, g.edges);

        std::vector< std::pair<int,int> > edge_list = edgelist_create(g.nodes, g.nindex, g.nlist);

        std::println("input graph edge_list: {} edges", edge_list.size());
    }
    return boost::report_errors();
}
