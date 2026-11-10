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
 *          |  X  |
 *          | / \ |
 *          |/   \|
 *          *-----*
 */

static ECLgraph createBasic1() {
    ECLgraph g{};

    g.nodes = 4;
    g.edges = 12;

    g.nindex = new int[5]  { 0, 3, 6, 9, 12 };
    g.nlist  = new int[12] { 1, 2, 3, 0, 2, 3, 0, 1, 3, 0, 1, 2 };

    return g;
}

/*
*           *     *
 *          |\
 *          | \
 *          |  *     *
 *          | / \
 *          |/   \
 *          *-----*
 */
static ECLgraph createBasic2() {
    ECLgraph g{};

    g.nodes = 6;
    g.edges = 10;

    g.nindex = new int[6]  { 0, 2, 5, 8, 10, 10 };
    g.nlist  = new int[10] { 1, 2, 0, 2, 3, 0, 1, 3, 1, 2 };

    return g;
}

void freeBasic(ECLgraph &g) {
    delete[] g.nindex;
    delete[] g.nlist;
}

static void writeGraph(ECLgraph &g, const char* output_file) {
    std::cout << "nodes: " <<  g.nodes << std::endl;
    std::cout << "edges: " <<  g.edges << std::endl;

    std::cout << "nindex values: ";
    for (int i = 0; i < g.nodes; i++) {
        std::cout << g.nindex[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "nlist values: ";
    for (int i = 0; i < 12; i++) {
        std::cout << g.nlist[i] << " ";
    }
    std::cout << std::endl;

    writeECLgraph(g, output_file);
}

static void runBasic(const char* output_file) {
    ECLgraph g = createBasic1();

    writeGraph(g, "basicGraph1.egr");

    ECLgraph g2 = createBasic2();

    writeGraph(g2, "basicGraph2.egr");

    freeECLgraph(g);
    freeECLgraph(g2);
}

int main(const int argc, char* argv[]) {
    std::cout << "Create Basic Graph v0.1" << std::endl;

    if (argc != 1) {fprintf(stderr, "USAGE: %s\n\n", argv[0]);  exit(-1);}

    runBasic(argv[1]);

    return 0;
}