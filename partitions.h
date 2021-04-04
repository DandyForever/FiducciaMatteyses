#ifndef HYPERGRAPHS_PARTITIONS_H
#define HYPERGRAPHS_PARTITIONS_H

#include <vector>
#include <ostream>
#include <ctime>
#include "hypergraph.h"

using namespace std;

struct Partitions {
    vector <bool> vert_partitions;
    size_t vert_num = 0;
    const Hypergraph* hg_;
    int solution_cost = 0;
    int balance = 0; //0: from left to right
    //1: from right to left
    const int tolerance = 1;
    int cost_check = 0;

    Partitions () = delete;
    Partitions (const Hypergraph& hg, bool is_static, int tolerance_percentage);

    int getCost () { return solution_cost; }
    bool is_cost_ok ();

    void update (size_t vert_id);

    void dump ();
    void dump (string& file_name);

private:
    void calculate_solution_cost();

    void static_init ();
    void random_init ();
};



#endif //HYPERGRAPHS_PARTITIONS_H
