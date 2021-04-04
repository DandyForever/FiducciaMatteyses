#ifndef HYPERGRAPHS_HYPERGRAPH_H
#define HYPERGRAPHS_HYPERGRAPH_H

#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

struct Hypergraph {
    vector <vector <int>> verts;
    vector <vector <int>> edges;
    size_t vert_num = 0;
    size_t edge_num = 0;

    ostream& logger;

    Hypergraph() = delete;
    explicit Hypergraph(const string& file_name, ostream& logger_);
};


#endif //HYPERGRAPHS_HYPERGRAPH_H
