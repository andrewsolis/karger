/*
ECL-CC code: ECL-CC is a connected components graph algorithm. It operates
on graphs stored in binary CSR format.

Copyright (c) 2017-2020, Texas State University. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of Texas State University nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL TEXAS STATE UNIVERSITY BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Authors: Jayadharini Jaiganesh and Martin Burtscher

URL: The latest version of this code is available at
https://userweb.cs.txstate.edu/~burtscher/research/ECL-CC/.

Publication: This work is described in detail in the following paper.
Jayadharini Jaiganesh and Martin Burtscher. A High-Performance Connected
Components Implementation for GPUs. Proceedings of the 2018 ACM International
Symposium on High-Performance Parallel and Distributed Computing, pp. 92-104.
June 2018.
*/

#include <random>
#include <unordered_set>
#include <vector>
// Set to true for debug builds, false for release
constexpr bool DEBUG_MODE = true;

#include <stdlib.h>
#include <stdio.h>
#include <set>
#include <algorithm>
#include <sys/time.h>
#include "ECLgraph.h"
#include "ECLKarger.h"

void init(const int nodes, const int* const __restrict__ nidx, const int* const __restrict__ nlist, int* const __restrict__ nstat)
{
  #pragma omp parallel for schedule(guided) default(none) shared(nodes, nidx, nlist, nstat)
  for (int v = 0; v < nodes; v++) {
    const int beg = nidx[v];
    const int end = nidx[v + 1];
    int m = v;
    int i = beg;
    while ((m == v) && (i < end)) {
      m = std::min(m, nlist[i]);
      i++;
    }
    nstat[v] = m;
  }
}

static inline int representative(const int idx, int* const __restrict__ nstat)
{
  int curr = nstat[idx];
  if (curr != idx) {
    int next, prev = idx;
    while (curr > (next = nstat[curr])) {
      nstat[prev] = next;
      prev = curr;
      curr = next;
    }
  }
  return curr;
}

void compute(const int nodes, const int* const __restrict__ nidx, const int* const __restrict__ nlist, int* const __restrict__ nstat)
{
  #pragma omp parallel for schedule(guided) default(none) shared(nodes, nidx, nlist, nstat)
  for (int v = 0; v < nodes; v++) {
    const int vstat = nstat[v];
    if (v != vstat) {
      const int beg = nidx[v];
      const int end = nidx[v + 1];
      int vstat = representative(v, nstat);
      for (int i = beg; i < end; i++) {
        const int nli = nlist[i];
        if (v > nli) {
          int ostat = representative(nli, nstat);
          bool repeat;
          do {
            repeat = false;
            if (vstat != ostat) {
              int ret;
              if (vstat < ostat) {
                if ((ret = __sync_val_compare_and_swap(&nstat[ostat], ostat, vstat)) != ostat) {
                  ostat = ret;
                  repeat = true;
                }
              } else {
                if ((ret = __sync_val_compare_and_swap(&nstat[vstat], vstat, ostat)) != vstat) {
                  vstat = ret;
                  repeat = true;
                }
              }
            }
          } while (repeat);
        }
      }
    }
  }
}

void flatten(const int nodes, int* const __restrict__ nstat)
{
  #pragma omp parallel for default(none) shared(nodes, nstat)
  for (int v = 0; v < nodes; v++) {
    int next, vstat = nstat[v];
    const int old = vstat;
    while (vstat > (next = nstat[vstat])) {
      vstat = next;
    }
    if (old != vstat) nstat[v] = vstat;
  }
}

static void verify(const int v, const int id, const int* const __restrict__ nidx, const int* const __restrict__ nlist, int* const __restrict__ nstat)
{
  if (nstat[v] >= 0) {
    if (nstat[v] != id) {fprintf(stderr, "ERROR: found incorrect ID value\n\n");  exit(-1);}
    nstat[v] = -1;
    for (int i = nidx[v]; i < nidx[v + 1]; i++) {
      verify(nlist[i], id, nidx, nlist, nstat);
    }
  }
}

static void printData(const int nodes,
                      const int* const __restrict__ nidx,
                      const int* const __restrict__ nlist,
                      const int* const __restrict__ nstat,
                      const std::vector< std::pair<int, int> >& edgelist) {
  printf("nodeID   nidx    status\n");
  for (int i = 0; i < nodes; ++i) {
      printf("%d \t %d \t %d\n", i, nidx[i], nstat[i]);
  }

  printf("nodelist: ");
  for (int v = 0; v < nodes; ++v) {
    const int beg = nidx[v];
    const int end = nidx[v + 1];

    for (int i=beg; i < end; i++) {
      printf("%d, ", nlist[i]);
    }
  }
  printf("\n");

  printf("Edges: ");

  for (int v = 0; v < nodes; ++v) {
    const int beg = nidx[v];
    const int end = nidx[v + 1];

    for (int i=beg; i < end; i++) {
      printf("(%d, %d) ", v, nlist[i]);
    }
  }
  printf("\n");

  printf("Edgelist: ");

  for (const auto & it : edgelist) {
    printf("(%d, %d) ", it.first, it.second);
  }
  printf("\n");
}

static std::vector<std::pair<int, int>> getedgelist(const int nodes,
                                                 const int* const __restrict__ nidx,
                                                 int * const __restrict__ nlist) {
  std::set< std::pair<int, int> > edgeset;

  for (int v = 0; v < nodes; ++v) {
    const int beg = nidx[v];
    const int end = nidx[v + 1];

    for (int i=beg; i < end; i++) {
      int first = std::min(v, nlist[i]);
      int second = std::max(v, nlist[i]);
      std::pair<int,int> edge(first, second);
      edgeset.insert(edge);
    }
  }

  std::vector edgelist(edgeset.begin(), edgeset.end() );
  return edgelist;
}

static void printpermutation(const std::vector< std::pair<int, int> >& edgelist) {

  printf("edge permutation: ");
  for (const auto & it : edgelist) {
    printf("(%d, %d) ", it.first, it.second);
  }

  printf("\n");
}

static void printedges(const std::vector<std::pair<int, int>>& edgelist) {
  printf("edges: ");
  for (const std::pair edge : edgelist){
    printf("(%d, %d) ", edge.first, edge.second);
  }

  printf("\n");
}

int main(int argc, char* argv[])
{
  printf("ECLKarger v0.1 OpenMP (%s)\n", __FILE__);
  printf("Copyright 2025-2026 Texas State University & University of Texas at Austin\n");

  if (argc != 4) {fprintf(stderr, "USAGE: %s input_file_name num_permutations seed\n\n", argv[0]);  exit(-1);}

  ECLgraph g = readECLgraph(argv[1]);
  const int numpermutations = std::stoi(argv[2]);
  const int seed = std::stoi(argv[3]);
  int* const nodestatus = new int [g.nodes];

  printf("input graph: %d nodes and %d edges (%s)\n", g.nodes, g.edges, argv[1]);
  printf("average degree: %.2f edges per node\n", 1.0 * g.edges / g.nodes);
  int mindeg = g.nodes;
  int maxdeg = 0;
  for (int v = 0; v < g.nodes; v++) {
    int deg = g.nindex[v + 1] - g.nindex[v];
    mindeg = std::min(mindeg, deg);
    maxdeg = std::max(maxdeg, deg);
  }
  printf("minimum degree: %d edges\n", mindeg);
  printf("maximum degree: %d edges\n", maxdeg);


  // get initial edgelist
  std::vector< std::pair<int, int> > edgelist = getedgelist(g.nodes, g.nindex, g.nlist);

  // create array for storing nindex as it is updated
  int* newnindex = new int[g.nodes + 1];

  if constexpr (DEBUG_MODE) {
    printData(g.nodes, g.nindex, g.nlist, nodestatus, edgelist);
  }

  // create random generator
  std::random_device rd;
  std::mt19937 gen(1337);

  // iterate for number of permutations
  for (int p = 0; p < numpermutations; p++ ) {
    int* newnlist = new int[edgelist.size() * 2];

    // shuffle initial edgelist
    std::ranges::shuffle(edgelist, gen);
    if constexpr (DEBUG_MODE) {
      printpermutation(edgelist);
    }

    // get first half of edges
    long initend = std::ssize(edgelist) / 2;;
    std::vector edges(edgelist.begin(), (edgelist.begin() + initend) );

    if constexpr (DEBUG_MODE) {
      printedges(edges);
    }

    bool found = false;
    while (!found) {
      
      // build new nlist and nindex arrays for graph
      int start = 0, last = 0;
      for (int i = 0; i < g.nodes; i++) {
        newnindex[i] = start;
        for (int j = 0; j < g.nodes; j++ ) {

          int first = std::min(i, j);
          int second = std::max(i, j);

          std::pair t(first, second);
          if (std::ranges::find(edges, t) != edges.end()) {
            newnlist[last] = j;
            last++;
          }
        }

        start = last;
      }

      newnindex[g.nodes] = last;

      printf("newnindex: [");
      for (int i = 0; i < (g.nodes + 1); i++) {
        printf("%d,",newnindex[i]);
      }

      printf("]\n");

      printf("nodelist: ");
      for (int v = 0; v < g.nodes; ++v) {
        const int beg = newnindex[v];
        const int end = newnindex[v + 1];

        for (int i=beg; i < end; i++) {
          printf("%d, ", newnlist[i]);
        }
      }
      printf("\n");

      // generate new nnodestatus array to store cc
      int* const newnodestatus = new int [g.nodes];

      // test new nlist and nindex
      init(g.nodes, newnindex, newnlist, newnodestatus);
      compute(g.nodes, newnindex, newnlist, newnodestatus);
      flatten(g.nodes, newnodestatus);

      // verify results

      std::set<int> s1;
      for (int v = 0; v < g.nodes; v++) {
        s1.insert(newnodestatus[v]);
      }
      printf("number of connected components: %d\n", (int)s1.size());

      for (int v = 0; v < g.nodes; v++) {
        for (int i = newnindex[v]; i < newnindex[v + 1]; i++) {
          if (newnodestatus[newnlist[i]] != newnodestatus[v]) {fprintf(stderr, "ERROR: found adjacent nodes in different components\n\n");  exit(-1);}
        }
      }

      for (int v = 0; v < g.nodes; v++) {
        if (newnodestatus[v] < 0) {fprintf(stderr, "ERROR: found negative component number\n\n");  exit(-1);}
      }

      std::set<int> s2;
      int count = 0;
      for (int v = 0; v < g.nodes; v++) {
        if (newnodestatus[v] >= 0) {
          count++;
          s2.insert(newnodestatus[v]);
          verify(v, newnodestatus[v], newnindex, newnlist, newnodestatus);
        }
      }
      if (s1.size() != s2.size()) {fprintf(stderr, "ERROR: number of components do not match\n\n");  exit(-1);}
      if (s1.size() != (unsigned)count) {fprintf(stderr, "ERROR: component IDs are not unique\n\n");  exit(-1);}

      delete[] newnlist;
      delete[] newnodestatus;
    }

  }

  delete[] newnindex;

  delete [] nodestatus;
  return 0;
}