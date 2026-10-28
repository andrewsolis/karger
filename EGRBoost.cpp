//
// Created by Andrew Solis on 5/5/2026.
//

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <print>
#include <ranges>
#include <chrono>

#include <boost/graph/adjacency_list.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>

#include "mas_sw_maxflow_oracle.hpp"
#include "karger_helper.h"
#include "ECLgraph.h"

// Set compile time constant based on macro for debug mode
#ifdef DEBUG
constexpr bool debug_mode = true;
#else
constexpr bool debug_mode = false;
#endif

using mas_sw_oracle::weight_type;

using undirected_unweighted_graph = boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS >;

std::string egr_path;

struct edge_t
{
    unsigned long first;
    unsigned long second;
};

int main(int argc, char* argv[])
{

    if (BOOST_TEST(argc == 2))
    {
        // read in egr path
        egr_path = argv[1];

        std::println("EGR file path: {}", egr_path);

        // read in file as ecl egr graph
        ECLgraph ecl_g = readECLgraph(argv[1]);

        // print out nodes and edges.
        // NOTE: edges are duplicated so edge count is doubled
        std::println("input graph: {} nodes and {} edges(duplicates x 2)", ecl_g.nodes, ecl_g.edges);

        if constexpr (debug_mode) {
            std::println("creating edge list");
        }

        // call helper function to create edge list with duplicates removed
        std::vector< std::pair<int,int> > edge_list = edgelist_create(ecl_g.nodes, ecl_g.nindex, ecl_g.nlist);

        // print out final edge list count with duplicates removed
        std::println("input graph edge_list: {} edges", edge_list.size());

        // setup taken from test4() of stoer_wagner_test.cpp
        // -------------------------------------------------

        // Initialize vertex and edge descriptors
        using vd = boost::graph_traits< undirected_unweighted_graph >::vertex_descriptor;
        using ed = boost::graph_traits< undirected_unweighted_graph >::edge_descriptor;

        // create edges array for boost to use
        edge_t edges[ edge_list.size() ];

        if constexpr (debug_mode) {
            std::println("copying edgelist to edges array");
        }

        // add edges from vector to edge list array
        for (size_t i = 0; i < edge_list.size(); i++) {
            edges[i] = edge_t(edge_list[i].first, edge_list[i].second);
        }

        if constexpr (debug_mode) {
            std::println("creating undirected unweighted graph");
        }

        // create undirected unweighted graph from edge list
        const undirected_unweighted_graph boost_g(edges, edges + edge_list.size(), ecl_g.nodes);

        if constexpr (debug_mode) {
            std::println("creating parity property map");
        }

        // create map for each vector that sets it's parity. Those that are
        // true are in one group and those that are false are in another
        std::map< vd, bool > parity;
        boost::associative_property_map< std::map< vd, bool > > parities(parity);

        if constexpr (debug_mode) {
            std::println("Calling Stoer Wagner");
        }

        // starting runtime timer for stoer wager function
        std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

        // call stoer_wagner which returns weight of cut, and set
        // weight of each edge to 1
        const int w = boost::graph::stoer_wagner_min_cut(boost_g,
            boost::make_constant_property< ed >(weight_type(1)), parities);

        // stopping clock
        const std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

        auto time_span = duration_cast<std::chrono::duration<double>>(end - start);

        std::println("Stoer Wagner took {} seconds", time_span.count());

        // Since each weight is 1 then the resulting weight equals the number
        // of edges that were cut
        std::println("The min-cut weight of the graph is {}", w);

    }
    return boost::report_errors();
}
