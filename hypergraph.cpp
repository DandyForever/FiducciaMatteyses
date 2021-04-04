#include "hypergraph.h"

Hypergraph::Hypergraph(const string& file_name, ostream& logger_):
        logger(logger_)
{
    fstream hg_file;
    hg_file.open(file_name);
    if (!hg_file) {
        throw runtime_error("Could not open file");
    }

    int num_v = 0, num_e = 0;
    hg_file >> num_e >> num_v;
    verts = vector <vector <int>>(++num_v);
    edges = vector <vector <int>>(++num_e);
    vert_num = num_v - 1;
    edge_num = num_e - 1;
    logger << "Read num of vertecies " << num_v - 1 << " and edges " << num_e - 1 << "\nReserved vectors for them" << std::endl;
    string tmp_str;
    int edge_count = 0, vert_curr = 0;
    while (getline(hg_file, tmp_str)) {
        istringstream tmp_str_stm(tmp_str);
        while (tmp_str_stm >> vert_curr) {
            verts[vert_curr].push_back(edge_count);
            edges[edge_count].push_back(vert_curr);
        }
        edge_count++;
    }
    logger << "Read hypergraph from file. " << edge_count - 1 << " edges read" << std::endl;
    hg_file.close();
}

