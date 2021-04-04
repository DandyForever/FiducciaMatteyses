#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <map>
#include <set>
#include <ctime>
#include <cstdlib>
#include <unistd.h>

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
    const int tolerance = 1;
    int cost_check = 0;

    Partitions () = delete;
    Partitions (const Hypergraph& hg, bool is_static, int tolerance_percentage):
        tolerance(hg.vert_num * tolerance_percentage / 100 ? hg.vert_num * tolerance_percentage / 100 : 1)
    {
        vert_partitions = vector <bool>(hg.vert_num + 1);
        vert_num = hg.vert_num;
        if (is_static)
            static_init ();
        else
            random_init ();
        logger << "Initial partition done: " << vert_num / 2 << " verteces left and " << vert_num - vert_num / 2 << " vertices right" << std::endl;
        hg_ = &hg;
        calculate_solution_cost();
        balance = vert_num % 2;
    }

    int getCost () { return solution_cost; }

    bool is_cost_ok () {
        cost_check = solution_cost;
        solution_cost = 0;
        calculate_solution_cost();
        bool is_ok = cost_check == solution_cost;
        solution_cost = cost_check;
        return is_ok;
    }

    void update (size_t vert_id) {
        if (vert_partitions[vert_id])
            balance--;
        else
            balance++;
        vert_partitions[vert_id] = !vert_partitions[vert_id];
    }

    void dump () {
        int count_left = 0;
        for (int i = 1; i <= vert_num; i++) {
            if (!vert_partitions[i])
                count_left++;
        }
        logger << "Left: " << count_left << endl;
        int count_right = 0;
        for (int i = 1; i <= vert_num; i++) {
            if (vert_partitions[i])
                count_right++;
        }
        logger << "Right: " << count_right << endl;
        logger << "Solution cost: " << solution_cost << endl;
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

    void static_init () {
        for (size_t i = 1; i <= vert_num / 2; i++)
            vert_partitions[i] = false;
        for (size_t i = vert_num / 2 + 1; i <= vert_num; i++)
            vert_partitions[i] = true;
    }

    void random_init () {
        srand(time(NULL) % 100);
        int counter = 0;
        for (int i = 1; i <= vert_num; i++) {
            if (rand() % 2) {
                vert_partitions[i] = true;
                counter++;
            } else {
                vert_partitions[i] = false;
                counter--;
            }
        }
        int count = 1;
        while (counter > 1) {
            if (vert_partitions[count]) {
                vert_partitions[count] = false;
                counter -= 2;
            }
            count++;
        }
        while (counter < 0) {
            if (!vert_partitions[count]) {
                vert_partitions[count] = true;
                counter += 2;
            }
            count++;
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
//        GainContainer gc_cosim(partitions, 0);
        vert_gain = vector <int> (partitions.vert_num + 1, 0);
        for (size_t e = 1; e <= partitions.hg_->edge_num; e++) {
            int vert_left = 0, vert_right = 0, vertex_id_left = 0, vertex_id_right = 0;

            for (size_t i = 0; i < partitions.hg_->edges[e].size(); i++) {
                int vert_id = partitions.hg_->edges[e][i];

                if (!partitions.vert_partitions[vert_id]) {
                    vert_left++;
                    vertex_id_left = vert_id;
                } else {
                    vert_right++;
                    vertex_id_right = vert_id;
                }
            }

            if ((vert_right == 0 && vert_left != 1) || (vert_left == 0 && vert_right != 1)) {
                for (size_t i = 0; i < partitions.hg_->edges[e].size(); i++) {
                    int vert_id = partitions.hg_->edges[e][i];

                    vert_gain[vert_id]--;
                }
            }
            if (vert_left == 1 && vert_right != 0)
                vert_gain[vertex_id_left]++;
            if (vert_right == 1 && vert_left != 0)
                vert_gain[vertex_id_right]++;
        }
        for (size_t v = 1; v <= partitions.vert_num; v++) {
            if (!partitions.vert_partitions[v])
                left[vert_gain[v]].insert(v);
            else
                right[vert_gain[v]].insert(v);
        }
//        for (size_t i = 1; i <= partitions.vert_num; i++) {
//            if (vert_gain[i] != gc_cosim.vert_gain[i]) {
//                cout << "DEBUG: Different gain for " << i << " calculated " << vert_gain[i] << " real " << gc_cosim.vert_gain[i] << endl;
//            }
//        }
        logger << "Gain container initialized" << std::endl;
    }
    GainContainer (const Partitions& partitions, bool flag) {
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
        return left.empty() || right.empty();
    }

    pair <size_t, int> best_feasible_move (int balance, int tolerance) {
        auto gain_verts_right = --right.end();
        auto gain_verts_left = --left.end();
        int gain_right = gain_verts_right->first;
        int gain_left = gain_verts_left->first;
        bool is_right = false;
        if ((gain_left < gain_right && -balance < tolerance) || balance >= tolerance)
            is_right = true;

        if (is_right) {
            size_t vert = *((gain_verts_right->second).begin());
            (gain_verts_right->second).erase(vert);
            if ((gain_verts_right->second).empty())
                right.erase(gain_right);
            return {vert, gain_right};
        }
        size_t vert = *((gain_verts_left->second).begin());
        (gain_verts_left->second).erase(vert);
        if ((gain_verts_left->second).empty())
            left.erase(gain_left);
        return {vert, gain_left};
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

            if (partitionment.vert_partitions[best_move.first]) { //DST is left
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
        auto best_move = gc.best_feasible_move(partitionment.balance, partitionment.tolerance);
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

size_t FM (Partitions& partitions) {
    bool is_improved = true;
    size_t epoch = 0;
    while (is_improved) {
        logger << "Epoch #" << epoch++ << endl;
        partitions.dump();
        size_t gc_init_start_t = clock();
        GainContainer gc(partitions);
        size_t gc_init_end_t = clock();
        if (partitions.is_cost_ok())
            logger << "Cost is OK" <<endl;
        else
            logger << "Cost is not OK" << endl;
        size_t cost_check_end_t = clock();
        int best = FMpass(gc, partitions);
        if (best == partitions.solution_cost)
            is_improved = false;
        else
            partitions.solution_cost = best;
        size_t pass_end_t = clock();
        logger << "Current solution cost is " << best << " balance " << partitions.balance << endl;
        size_t gc_init = (gc_init_end_t - gc_init_start_t) / 1000;
        size_t check = (cost_check_end_t - gc_init_end_t) / 1000;
        size_t pass = (pass_end_t - cost_check_end_t) / 1000;
        size_t epoch_t = (pass_end_t - gc_init_start_t) / 1000;
        logger << "GC init: " << gc_init << " s\nCost check: " << check << " s\nFM pass: " << pass << " s\nTotal: " << epoch_t << endl << endl;
    }
    return epoch;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cout << "File not found" << endl;
        return -1;
    }
    int tolerance_percentage = 0;
    if (argc == 3) {
        tolerance_percentage = atoi (argv[2]);
    }
    string tests[18] = {
            "..\\benchmarks\\ISPD98_ibm01.hgr",
            "..\\benchmarks\\ISPD98_ibm02.hgr",
            "..\\benchmarks\\ISPD98_ibm03.hgr",
            "..\\benchmarks\\ISPD98_ibm04.hgr",
            "..\\benchmarks\\ISPD98_ibm05.hgr",
            "..\\benchmarks\\ISPD98_ibm06.hgr",
            "..\\benchmarks\\ISPD98_ibm07.hgr",
            "..\\benchmarks\\ISPD98_ibm08.hgr",
            "..\\benchmarks\\ISPD98_ibm09.hgr",
            "..\\benchmarks\\ISPD98_ibm10.hgr",
            "..\\benchmarks\\ISPD98_ibm11.hgr",
            "..\\benchmarks\\ISPD98_ibm12.hgr",
            "..\\benchmarks\\ISPD98_ibm13.hgr",
            "..\\benchmarks\\ISPD98_ibm14.hgr",
            "..\\benchmarks\\ISPD98_ibm15.hgr",
            "..\\benchmarks\\ISPD98_ibm16.hgr",
            "..\\benchmarks\\ISPD98_ibm17.hgr",
            "..\\benchmarks\\ISPD98_ibm18.hgr"
    };
    string extra_tests[10] = {
            "..\\benchmarks\\dac2012_superblue2.hgr",
            "..\\benchmarks\\dac2012_superblue3.hgr",
            "..\\benchmarks\\dac2012_superblue6.hgr",
            "..\\benchmarks\\dac2012_superblue7.hgr",
            "..\\benchmarks\\dac2012_superblue9.hgr",
            "..\\benchmarks\\dac2012_superblue11.hgr",
            "..\\benchmarks\\dac2012_superblue12.hgr",//10903650
            "..\\benchmarks\\dac2012_superblue14.hgr",
            "..\\benchmarks\\dac2012_superblue16.hgr",
            "..\\benchmarks\\dac2012_superblue19.hgr",
    };
    for (int i = 0 ; i < 10; i++) {
        size_t start_t = clock();
        string in_file_name = extra_tests[i];//string(argv[1]);
        string out_file_name = in_file_name + ".part.2";
        string log_file_name = in_file_name + ".log";
        logger.open(log_file_name);
        logger << "Algorithm Feduccie Mattheyses" << endl;
        Hypergraph hg(in_file_name);
        Partitions partitions(hg, true, tolerance_percentage);
        size_t epoch = FM(partitions);
        size_t end_t = clock();
        size_t exec_t = (end_t - start_t) / 1000;
        partitions.dump(out_file_name);
        logger << "Total execution time is " << exec_t / 60 << " minutes " << exec_t % 60 << " seconds" << endl;
        logger.close();
        cout << "Test name: " << extra_tests[i] <<
                " Num of vertecies " << hg.vert_num <<
                " Num of edges " << hg.edge_num <<
                " balance " << partitions.balance <<
                " cut " << partitions.solution_cost <<
                " time " << exec_t <<
                " #iters " << epoch << endl;
    }
    return 0;
}

