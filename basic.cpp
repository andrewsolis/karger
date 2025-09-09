// system headers
#include <iostream>

// Custom headers
#include "ECLgraph.h"

/*
 * Create a basic ECLGraph where there are 4 nodes, each node is connected
 * to every other available node
 *
 *          *-----*
 *          |\   /|
 *          | \ / |
 *          |  x  |
 *          | / \ |
 *          |/   \|
 *          *-----*
 */
ECLgraph createBasic() {
    ECLgraph g{};

    g.nodes = 4;
    g.edges = 6;

    g.nindex = new int[4]  { 0, 3, 6, 9 };
    g.nlist  = new int[12] { 0, 1, 5, 0, 2, 4, 4, 1, 3, 5, 2, 3 };

    return g;
}

void freeBasic(ECLgraph &g) {
    delete[] g.nindex;
    delete[] g.nlist;
}

void runBasic(const char* output_file) {
    ECLgraph g = createBasic();

    std::cout << "nodes: " <<  g.nodes << std::endl;
    std::cout << "edges: " <<  g.edges << std::endl;

    std::cout << "nindex values: ";
    for (int i = 0; i < g.nodes; i++) {
        std::cout << g.nindex[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "nlist values: ";
    for (int i = 0; i < g.edges * 2; i++) {
        std::cout << g.nlist[i] << " ";
    }
    std::cout << std::endl;

    writeECLgraph(g, output_file);

    freeECLgraph(g);
}

int main(const int argc, char* argv[]) {
    std::cout << "Create Basic Graph v0.1" << std::endl;

    if (argc != 2) {fprintf(stderr, "USAGE: %s output_file_name\n\n", argv[0]);  exit(-1);}

    runBasic(argv[1]);

    return 0;
}