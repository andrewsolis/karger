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


#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <set>
#include <vector>
#include <sys/time.h>
#include "ECLgraph.h"

// check to see if edge exists in edgelist to be avoided
static bool edgeverify(int v1, int v2, const std::vector< std::pair<int, int> >& edgelist) {
  std::pair<int,int> edge = {std::min(v1, v2), std::max(v1, v2)};

  if (std::find(edgelist.begin(), edgelist.end(), edge) == edgelist.end()) {
    return false;
  }
  return true;
}

void init(const int nodes, const int* const __restrict__ nidx, const int* const __restrict__ nlist, int* const __restrict__ nstat, const std::vector< std::pair<int, int> >& edgelist)
{
  #pragma omp parallel for schedule(guided) default(none) shared(nodes, nidx, nlist, nstat)
  for (int v = 0; v < nodes; v++) {
    // const int beg = nidx[v];
    // const int end = nidx[v + 1];
    int m = v;
    // int i = beg;
    // while ((m == v) && (i < end)) {
    //
    //   if (!edgeverify(m, nlist[i], edgelist)){
    //     m = std::min(m, nlist[i]);
    //   }
    //   i++;
    // }
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

void compute(const int nodes, const int* const __restrict__ nidx, const int* const __restrict__ nlist, int* const __restrict__ nstat, const std::vector< std::pair<int, int> >& edgelist)
{
  #pragma omp parallel for schedule(guided) default(none) shared(nodes, nidx, nlist, nstat)
  for (int v = 0; v < nodes; v++) {
    const int vstat = nstat[v];
    if (v  != vstat) {
      const int beg = nidx[v];
      const int end = nidx[v + 1];
      int vstat = representative(v, nstat);
      for (int i = beg; i < end; i++) {

        const int nli = nlist[i];

        if (!edgeverify(v, nli, edgelist)){
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

static void verify(const int v, const int id, const int* const __restrict__ nidx, const int* const __restrict__ nlist, int* const __restrict__ nstat, const std::vector< std::pair<int, int> >& edgelist)
{
  if (nstat[v] >= 0) {
    if (nstat[v] != id) {fprintf(stderr, "ERROR: found incorrect ID value\n\n");  exit(-1);}
    nstat[v] = -1;
    for (int i = nidx[v]; i < nidx[v + 1]; i++) {

      if (!edgeverify(v, nlist[i], edgelist)){
        verify(nlist[i], id, nidx, nlist, nstat, edgelist);
      }
    }
  }
}

std::vector< std::pair<int, int> > permutation(const int nodes, int * const __restrict__ nidx, int * const __restrict__ nlist) {

  std::set< std::pair<int,int> > edgelist_set;
  for (int i = 0; i < nodes; i++) {

    const int beg = nidx[i];
    const int end = nidx[i + 1];

    for (int j = beg; j < end; j++) {

      int first = std::min(i, nlist[j]);
      int second = std::max(i, nlist[j]);
      std::pair<int,int> edge(first, second);
        edgelist_set.insert(edge);
    }

  }

  std::vector< std::pair<int,int> > edgelist_vec(edgelist_set.begin(), edgelist_set.end());
  return edgelist_vec;
}

int main(int argc, char* argv[])
{
  printf("ECL-CC v1.1 OpenMP (%s)\n", __FILE__);
  printf("Copyright 2017-2020 Texas State University\n");

  if (argc != 2) {fprintf(stderr, "USAGE: %s input_file_name\n\n", argv[0]);  exit(-1);}

  ECLgraph g = readECLgraph(argv[1]);
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

  // get initial permutation
  std::vector< std::pair<int,int> > edgelist = permutation(g.nodes, g.nindex, g.nlist);

  // only use half of vector for now
  std::vector< std::pair<int,int> > edgelist_half(edgelist.begin(), edgelist.begin() + edgelist.size() / 2);

  struct timeval start, end;
  gettimeofday(&start, NULL);

  init(g.nodes, g.nindex, g.nlist, nodestatus, edgelist_half);
  compute(g.nodes, g.nindex, g.nlist, nodestatus, edgelist_half);
  flatten(g.nodes, nodestatus);

  gettimeofday(&end, NULL);
  double runtime = end.tv_sec + end.tv_usec / 1000000.0 - start.tv_sec - start.tv_usec / 1000000.0;

  printf("compute time: %.4f s\n", runtime);
  printf("throughput: %.3f Mnodes/s\n", g.nodes * 0.000001 / runtime);
  printf("throughput: %.3f Medges/s\n", g.edges * 0.000001 / runtime);

  std::set<int> s1;
  for (int v = 0; v < g.nodes; v++) {
    s1.insert(nodestatus[v]);
  }
  printf("number of connected components: %d\n", (int)s1.size());


  for (int v = 0; v < g.nodes; v++) {
    for (int i = g.nindex[v]; i < g.nindex[v + 1]; i++) {

      if (!edgeverify(v, g.nlist[i], edgelist)){


        if (nodestatus[g.nlist[i]] != nodestatus[v]) {fprintf(stderr, "ERROR: found adjacent nodes in different components\n\n");  exit(-1);}
      }

    }
  }

  for (int v = 0; v < g.nodes; v++) {
    if (nodestatus[v] < 0) {fprintf(stderr, "ERROR: found negative component number\n\n");  exit(-1);}
  }


  std::set<int> s2;
  int count = 0;
  for (int v = 0; v < g.nodes; v++) {
    if (nodestatus[v] >= 0) {
      count++;
      s2.insert(nodestatus[v]);
      verify(v, nodestatus[v], g.nindex, g.nlist, nodestatus, edgelist_half);
    }
  }
  if (s1.size() != s2.size()) {fprintf(stderr, "ERROR: number of components do not match\n\n");  exit(-1);}
  if (s1.size() != (unsigned)count) {fprintf(stderr, "ERROR: component IDs are not unique\n\n");  exit(-1);}

  printf("all good\n\n");

  delete [] nodestatus;
  return 0;
}