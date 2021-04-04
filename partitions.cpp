#include "partitions.h"

Partitions::Partitions (const Hypergraph& hg, bool is_static, int tolerance_percentage):
        tolerance(hg.vert_num * tolerance_percentage / 100 ? hg.vert_num * tolerance_percentage / 100 : 1)
{
    vert_partitions = vector <bool>(hg.vert_num + 1);
    vert_num = hg.vert_num;
    if (is_static)
        static_init ();
    else
        random_init ();
    hg_ = &hg;
    hg_->logger << "Initial partition done: " << vert_num / 2 << " verteces left and " << vert_num - vert_num / 2 << " vertices right" << std::endl;
    calculate_solution_cost();
    balance = vert_num % 2;
}

bool Partitions::is_cost_ok () {
    cost_check = solution_cost;
    solution_cost = 0;
    calculate_solution_cost();
    bool is_ok = cost_check == solution_cost;
    solution_cost = cost_check;
    return is_ok;
}

void Partitions::update (size_t vert_id) {
    if (vert_partitions[vert_id])
        balance--;
    else
        balance++;
    vert_partitions[vert_id] = !vert_partitions[vert_id];
}

void Partitions::dump () {
    int count_left = 0;
    for (int i = 1; i <= vert_num; i++) {
        if (!vert_partitions[i])
            count_left++;
    }
    hg_->logger << "Left: " << count_left << endl;
    int count_right = 0;
    for (int i = 1; i <= vert_num; i++) {
        if (vert_partitions[i])
            count_right++;
    }
    hg_->logger << "Right: " << count_right << endl;
    hg_->logger << "Solution cost: " << solution_cost << endl;
}

void Partitions::dump (string& file_name) {
    ofstream out_file;
    out_file.open(file_name);
    for (int i = 1; i <= vert_num; i++) {
        out_file << vert_partitions[i] << "\n";
    }
    out_file.close();
}

void Partitions::calculate_solution_cost() {
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

void Partitions::static_init () {
    for (size_t i = 1; i <= vert_num / 2; i++)
        vert_partitions[i] = false;
    for (size_t i = vert_num / 2 + 1; i <= vert_num; i++)
        vert_partitions[i] = true;
}

void Partitions::random_init () {
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