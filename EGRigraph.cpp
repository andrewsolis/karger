/*
igraph library.
   Copyright (C) 2006-2012  Gabor Csardi <csardi.gabor@gmail.com>
   334 Harvard street, Cambridge, MA 02139 USA

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc.,  51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA

*/

#include <print>
#include <vector>
#include <chrono>

#include <igraph.h>
#include "karger_helper.h"
#include "ECLgraph.h"

// Set compile time constant based on macro for debug mode
#ifdef DEBUG
constexpr bool debug_mode = true;
#else
constexpr bool debug_mode = false;
#endif


// struct for accessing egr graph edges
struct edge_t
{
    unsigned long first;
    unsigned long second;
};

int main(int argc, char* argv[])
{

    if (argc != 2) {std::println("Usage: {} egr_file", argv[0] ); exit(-1);}


    std::string egr_path;

    ECLgraph ecl_g{};

    // Declare graph, edges, and weight vector objects
    igraph_t g;
    igraph_vector_int_t edges;

    // vectors to store partition and edges cut
    igraph_vector_int_t partition1, partition2, cut;

    // value to store number of edges cut
    igraph_real_t value;


    // read in egr path
    egr_path = argv[1];

    std::println("EGR file path: {}", egr_path);

    // read in file as egr graph
    ecl_g = readECLgraph(egr_path.c_str());

    // print out nodes and edges.
    // NOTE: edges are duplicated so edge count is doubled
    std::println("input graph: {} nodes and {} edges(duplicates x 2)", ecl_g.nodes, ecl_g.edges);

    if constexpr (debug_mode) {
        std::println("creating edge list");
    }

    // call helper function to create edge list with duplicates removed
    const std::vector<std::pair<int, int> > edge_list = edgelist_create(ecl_g.nodes, ecl_g.nindex, ecl_g.nlist);

    // print out final edge list count with duplicates removed
    std::println("input graph edge_list: {} edges", edge_list.size());


    // Initialize igraph library
    igraph_setup();

    // Initialize vector of edges
    // multiply edge list by 2 since for igraph every 2 numbers is an edge
    const unsigned long edge_count = edge_list.size() * 2;

    igraph_vector_int_init(&edges, static_cast<igraph_int_t>(edge_count));

    // loop through vector and add edges
    for (size_t i = 0; i < edge_list.size(); i++) {
        VECTOR(edges)[i * 2] = edge_list[i].first;
        VECTOR(edges)[i * 2 + 1] = edge_list[i].second;
    }

    // create graph
    igraph_create(&g, &edges, ecl_g.nodes, IGRAPH_UNDIRECTED);

    // create vectors to store partition and edges cut
    igraph_vector_int_init(&partition1, 0);
    igraph_vector_int_init(&partition2, 0);
    igraph_vector_int_init(&cut, 0);

    // starting runtime timer for stoer wager function
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    // call mincut
    igraph_mincut(&g, &value, &partition1, &partition2, &cut, nullptr);

    // stopping clock
    const std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    auto time_span = duration_cast<std::chrono::duration<double>>(end - start);

    std::println("runtime: {} seconds", time_span.count() );

    std::println("mincut value: {}", value);

    // free partition and cut vectors
    igraph_vector_int_destroy(&cut);
    igraph_vector_int_destroy(&partition2);
    igraph_vector_int_destroy(&partition1);

    // free memory for edges vector and graph
    igraph_vector_int_destroy(&edges);
    igraph_destroy(&g);

    return 0;

}