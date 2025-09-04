// system headers
#include <iostream>
#include <unordered_set>

// Custom headers


#include "ECLgraph.h"


void karger(const ECLgraph g) {
    std::cout << "karger" << std::endl;

    // TODO: Calculate unique values inside edges array
    std::unordered_set<int> unique_edges;

    constexpr int edges_size = sizeof(&g.nlist) / sizeof(g.nlist[0]);

    for(int i = 0; i < edges_size; i++)
    {
        unique_edges.insert( g.nlist[ i ] );
    }

    std::cout << "unique_edges: " << unique_edges.size() << std::endl;
    for(int i = 0; i < unique_edges.size(); i++)
    {

    }
    // TODO: generate random permutations of edges


    // TODO: remove all edges from the first half of permutation
    // TODO: If two vertices left than only two connected components, else keep looking
}

int main(const int argc, char* argv[]) {


    std::cout << "Karger's Algorithm v0.1" << std::endl;

    // check command line
    if (argc != 2) {
        std::cerr << "USAGE: " << argv[0] << " input_file" << std::endl;
        exit(-1);
    }

    // read input
    ECLgraph g = readECLgraph(argv[1]);

    std::cout << "input: " <<  argv[1] << std::endl;
    std::cout << "nodes: " <<  g.nodes << std::endl;
    std::cout << "edges: " <<  g.edges << std::endl;

    karger( g );

    // clean up
    freeECLgraph(g);

    return 0;
}