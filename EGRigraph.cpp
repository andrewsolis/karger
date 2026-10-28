//
// Created by Andrew Solis on 10/28/26.
//

#include "EGRigraph.h"

#include <iostream>
#include <igraph.h> // Include the core igraph library

int main() {
    // Initialize the library if required by your build
    igraph_setup();

    igraph_t graph;

    // Create a ring graph with 10 vertices
    igraph_ring(&graph, 10, IGRAPH_UNDIRECTED, /* mutual = */ 0, /* circular = */ 1);

    std::cout << "Graph created with " << igraph_vcount(&graph) << " vertices.\n";

    // CRITICAL: You must manually free memory in C++ to prevent memory leaks
    igraph_destroy(&graph);
    return 0;
}