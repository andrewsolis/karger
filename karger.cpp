#include <iostream>

// Custom headers
#include "ECLgraph.h"

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

    // clean up
    freeECLgraph(g);

    return 0;
}