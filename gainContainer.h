#ifndef HYPERGRAPHS_GAINCONTAINER_H
#define HYPERGRAPHS_GAINCONTAINER_H

#include <set>
#include <map>
#include <vector>
#include <list>
#include <iostream>
#include "partitions.h"

using namespace std;

struct GainContainer {
    map <int, list <int>> left;
    map <int, list <int>> right;
    vector <int> vert_gain;
    vector <int> deltas;
    vector <std::list<int>::iterator> iterators;
    vector <bool> is_deleted;

    GainContainer () = delete;
    GainContainer (const Partitions& partitions);
    GainContainer (const Partitions& partitions, bool flag);

    bool is_empty (bool balance) const {
        return left.empty() || right.empty();
    }

    pair <size_t, int> best_feasible_move (int balance, int tolerance);

    void update (size_t vert_id, bool side, int delta);
    void erase (size_t vert_id, bool side);

    void dump ();
};

#endif //HYPERGRAPHS_GAINCONTAINER_H
