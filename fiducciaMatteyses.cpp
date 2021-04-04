#include "fiducciaMatteyses.h"

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
        partitions.hg_->logger << "Epoch #" << epoch++ << endl;
        partitions.dump();
        GainContainer gc(partitions);
        if (partitions.is_cost_ok())
            partitions.hg_->logger << "Cost is OK" <<endl;
        else
            partitions.hg_->logger << "Cost is not OK" << endl;
        int best = FMpass(gc, partitions);
        if (best == partitions.solution_cost)
            is_improved = false;
        else
            partitions.solution_cost = best;
        partitions.hg_->logger << "Current solution cost is " << best << " balance " << partitions.balance << endl << endl;
    }
    return epoch;
}

