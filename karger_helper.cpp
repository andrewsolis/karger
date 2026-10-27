//
// Created by Andrew Solis on 10/27/26.
//

#include <set>
#include <algorithm>
#include "karger_helper.h"


std::vector< std::pair<int, int> > edgelist_create(const int nodes, int * const __restrict__ nidx, int * const __restrict__ nlist) {

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
