#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <map>
#include <set>
#include <ctime>

using namespace std;

ofstream logger;

struct Hypergraph {
    vector <vector <int>> verts;
    vector <vector <int>> edges;
    size_t vert_num = 0;
    size_t edge_num = 0;

    Hypergraph() = delete;
    explicit Hypergraph(const string& file_name) {
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
};

struct Partitions {
    vector <bool> vert_partitions;
    size_t vert_num = 0;
    const Hypergraph* hg_;
    int solution_cost = 0;
    int balance = 0; //0: from left to right
                     //1: from right to left

    Partitions () = delete;
    Partitions (const Hypergraph& hg) {
        vert_partitions = vector <bool>(hg.vert_num + 1);
        vert_num = hg.vert_num;
        for (size_t i = 1; i <= vert_num / 2; i++)
            vert_partitions[i] = false;
        for (size_t i = vert_num / 2 + 1; i <= vert_num; i++)
            vert_partitions[i] = true;
        logger << "Initial partition done: " << vert_num / 2 << " verteces left and " << vert_num - vert_num / 2 << " vertices right" << std::endl;
        hg_ = &hg;
        calculate_solution_cost();
        balance = vert_num % 2;
    }

    int getCost () { return solution_cost; }

    void update (size_t vert_id) {
        vert_partitions[vert_id] = !vert_partitions[vert_id];
        balance = balance ? 0 : 1;
    }

    void dump () {
        cout << "Left: ";
        for (int i = 1; i <= vert_num; i++) {
            if (!vert_partitions[i])
                cout << i << " ";
        }
        cout << "\nRight: ";
        for (int i = 1; i <= vert_num; i++) {
            if (vert_partitions[i])
                cout << i << " ";
        }
        cout << "\nSolution cost: " << solution_cost << endl;
    }

    void dump (string& file_name) {
        ofstream out_file;
        out_file.open(file_name);
        for (int i = 1; i <= vert_num; i++) {
            out_file << vert_partitions[i] << "\n";
        }
        out_file.close();
    }

private:
    void calculate_solution_cost() {
        for (size_t i = 1; i < hg_->edges.size(); i++) {
            bool current_partition = vert_partitions[hg_->edges[i][0]];
            for (size_t j = 1; j < hg_->edges[i].size(); j++) {
                if (current_partition != vert_partitions[hg_->edges[i][j]]){
                    solution_cost++;
                    break;
                }
            }
        }
    }
};

struct GainContainer {
    map <int, set <int>> left;
    map <int, set <int>> right;
    vector <int> vert_gain;
    set <size_t> is_deleted;

    GainContainer () = delete;
    GainContainer (const Partitions& partitions) {
        vert_gain = vector <int> (partitions.vert_num + 1);
        for (size_t i = 1; i <= partitions.vert_num; i++) {
            int gain_current = 0;
            bool partition_current = partitions.vert_partitions[i];

            for (size_t j = 0; j < partitions.hg_->verts[i].size(); j++) {
                size_t edge_id = partitions.hg_->verts[i][j];

                bool is_alone_in_partition = true;
                bool is_entirely_in_partition = true;

                for (size_t v = 0; v < partitions.hg_->edges[edge_id].size(); v++) {
                    size_t vert_id = partitions.hg_->edges[edge_id][v];
                    if (partition_current != partitions.vert_partitions[vert_id])
                        is_entirely_in_partition = false;
                    if (partition_current == partitions.vert_partitions[vert_id] && vert_id != i)
                        is_alone_in_partition = false;
                }
                if (is_alone_in_partition)
                    gain_current++;

                if (is_entirely_in_partition)
                    gain_current--;
            }
            if (!partition_current)
                left[gain_current].insert(i);
            else
                right[gain_current].insert(i);
            vert_gain[i] = gain_current;
        }
        logger << "Gain container initialized" << std::endl;
    }

    bool is_empty (bool balance) const {
        return left.empty() && right.empty() || balance && right.empty() || !balance && left.empty();
    }

    pair <size_t, int> best_feasible_move (int balance) {
        if (balance) {
            auto gain_verts = --right.end();
            int gain = gain_verts->first;
            size_t vert = *((gain_verts->second).begin());
            (gain_verts->second).erase(vert);
            if ((gain_verts->second).empty())
                right.erase(gain);
            return {vert, gain};
        }
        auto gain_verts = --left.end();
        int gain = gain_verts->first;
        size_t vert = *((gain_verts->second).begin());
        (gain_verts->second).erase(vert);
        if ((gain_verts->second).empty())
            left.erase(gain);
        return {vert, gain};
    }

    void update (size_t vert_id, bool side, int delta) {
        if (is_deleted.count(vert_id))
            return;
        erase(vert_id, side);
        vert_gain[vert_id] += delta;
        if (side)
            right[vert_gain[vert_id]].insert(vert_id);
        else
            left[vert_gain[vert_id]].insert(vert_id);
    }

    void erase (size_t vert_id, bool side) {
        if (side) {
            right[vert_gain[vert_id]].erase(vert_id);
            if (right[vert_gain[vert_id]].empty())
                right.erase(vert_gain[vert_id]);
        } else {
            left[vert_gain[vert_id]].erase(vert_id);
            if (left[vert_gain[vert_id]].empty())
                left.erase(vert_gain[vert_id]);
        }
    }

    void dump () {
        std::cout << "Left" << std::endl;
        for (const auto& item : left) {
            std::cout << item.first << ": ";
            for (const auto& vert : item.second) {
                std::cout << vert << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "Right" << std::endl;
        for (const auto& item : right) {
            std::cout << item.first << ": ";
            for (const auto& vert : item.second) {
                std::cout << vert << " ";
            }
            std::cout << std::endl;
        }
    }
};

void applyMove (GainContainer& gc, Partitions& partitionment, const pair <size_t, int>& best_move) {
    for (size_t i = 0; i < partitionment.hg_->verts[best_move.first].size(); i++) {
        size_t edge_id = partitionment.hg_->verts[best_move.first][i];

        bool is_no_vert_in_dst = true;
        bool is_one_vert_in_src = true;

        int count_in_dst = 0;
        size_t vert_id_dst = 0;

        int count_in_src = 0;
        size_t vert_id_src = 0;
        for (size_t j = 0; j < partitionment.hg_->edges[edge_id].size(); j++) {
            size_t vert_id = partitionment.hg_->edges[edge_id][j];

            if (partitionment.balance) { //DST is left
                if (!partitionment.vert_partitions[vert_id]) {
                    is_no_vert_in_dst = false;

                    count_in_dst++;
                    vert_id_dst = vert_id;
                } else if (vert_id != best_move.first) {
                    count_in_src++;
                    vert_id_src = vert_id;
                }
            } else {                     //DST is right
                if (partitionment.vert_partitions[vert_id]) {
                    is_no_vert_in_dst = false;

                    count_in_dst++;
                    vert_id_dst = vert_id;
                } else if (vert_id != best_move.first) {
                    count_in_src++;
                    vert_id_src = vert_id;
                }
            }

            if (partitionment.vert_partitions[best_move.first] == partitionment.vert_partitions[vert_id] && vert_id != best_move.first)
                is_one_vert_in_src = false;
        }

        if (is_no_vert_in_dst) {
            for (size_t j = 0; j < partitionment.hg_->edges[edge_id].size(); j++) {
                size_t vert_id = partitionment.hg_->edges[edge_id][j];

                gc.update(vert_id, partitionment.vert_partitions[vert_id], 1);
            }
        }

        if (is_one_vert_in_src) {
            for (size_t j = 0; j < partitionment.hg_->edges[edge_id].size(); j++) {
                size_t vert_id = partitionment.hg_->edges[edge_id][j];

                gc.update(vert_id, partitionment.vert_partitions[vert_id], -1);
            }
        }

        if (count_in_src == 1)
            gc.update(vert_id_src, partitionment.vert_partitions[vert_id_src], 1);
        if (count_in_dst == 1)
            gc.update(vert_id_dst, partitionment.vert_partitions[vert_id_dst], -1);
    }
    gc.erase(best_move.first, partitionment.vert_partitions[best_move.first]);
    gc.is_deleted.insert(best_move.first);
    partitionment.update(best_move.first);
}

int FMpass (GainContainer& gc, Partitions& partitionment) {
    int solution_cost = partitionment.getCost();
    int best_cost = solution_cost;
    set <size_t> vert_to_change;

    while (!gc.is_empty(partitionment.balance)) {
        auto best_move = gc.best_feasible_move(partitionment.balance);
        vert_to_change.insert(best_move.first);
        solution_cost -= best_move.second;
        if (solution_cost < best_cost) {
            best_cost = solution_cost;
            vert_to_change.erase(vert_to_change.begin(), vert_to_change.end());
        }
        applyMove(gc, partitionment, best_move);
    }
    for (auto item : vert_to_change)
        partitionment.update(item);
    return best_cost;
}

void FM (Partitions& partitions) {
    bool is_improved = true;
    size_t epoch = 0;
    while (is_improved) {
        logger << "Epoch #" << epoch++ << endl;
        GainContainer gc(partitions);
        //gc.dump();
        //std::cout << "Solution cost of initial partition is " << partitions.getCost() << std::endl;
        int best = FMpass(gc, partitions);
        if (best == partitions.solution_cost)
            is_improved = false;
        else
            partitions.solution_cost = best;
        logger << "Current solution cost is " << best << " balance " << partitions.balance << endl << endl;
    }
}

//"..\\benchmarks\\ISPD98_ibm18.hgr"
//"..\\simple.hgr"
int main()
{
    size_t start_t = clock();
    string in_file_name = "..\\benchmarks\\ISPD98_ibm01.hgr";
    string out_file_name = in_file_name + ".part.2";
    string log_file_name = in_file_name + ".log";
    logger.open(log_file_name);
    logger << "Algorithm Fiduccia Matteyses" << endl;
    Hypergraph hg(in_file_name);
    Partitions partitions(hg);
    FM(partitions);
    size_t end_t = clock();
    size_t exec_t = (end_t - start_t) / 1000;
    partitions.dump(out_file_name);
    logger << "Total execution time is " << exec_t / 60 << " minutes " << exec_t % 60 << " seconds" << endl;
    logger.close();
    return 0;
}

