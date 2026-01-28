#include <cstdlib>
#include <cstdio>
#include "ECLgraph.h"


int main(int argc, char* argv [])
{
  printf("Convert ECL graph to dot format (%s)\n", __FILE__);
  printf("Copyright 2024 Texas State University\n\n");

  // process command line
  if (argc != 3) {fprintf(stderr, "USAGE: %s input_graph output_graph\n", argv[0]); exit(-1);}

  // read graph
  ECLgraph g = readECLgraph(argv[1]);
  printf("input: %s\n", argv[1]);
  printf("nodes: %d\n", g.nodes);
  printf("edges: %d (%d)\n\n", g.edges / 2, g.edges);

  // check size
  if (g.nodes > 100) {
    fprintf(stderr, "ERROR: input_graph has more than 100 nodes\n");
    freeECLgraph(g);
    exit(-1);
  }

  // output dot file
  FILE* f = fopen(argv[2], "w");
  fprintf(f, "graph {\n");
  for (int i = 0; i < g.nodes; i++) {
    fprintf(f, "  %d [style=filled, fillcolor=\"#d0d0d0\", label=\"%d\"]\n", i, i);
  }
  for (int i = 0; i < g.nodes; i++) {
    for (int j = g.nindex[i]; j < g.nindex[i + 1]; j++) {
      const int n = g.nlist[j];
      if (i < n) {
        fprintf(f, "  %d -- %d\n", i, n);
      }
    }
  }
  fprintf(f, "}\n");
  fclose(f);

  // clean up
  freeECLgraph(g);
  return 0;
}


/*
./ecl2dot graph.egr graph.dot
dot -Tpng graph.dot > graph.png
*/
