#include "gainContainer.h"

GainContainer::GainContainer (const Partitions& partitions) {
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
    partitions.hg_->logger << "Gain container initialized" << endl;
}

GainContainer::GainContainer (const Partitions& partitions, bool flag) {
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
    partitions.hg_->logger << "Gain container initialized" << endl;
}

pair <size_t, int> GainContainer::best_feasible_move (int balance, int tolerance) {
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

void GainContainer::update (size_t vert_id, bool side, int delta) {
    if (is_deleted.count(vert_id))
        return;
    erase(vert_id, side);
    vert_gain[vert_id] += delta;
    if (side)
        right[vert_gain[vert_id]].insert(vert_id);
    else
        left[vert_gain[vert_id]].insert(vert_id);
}

void GainContainer::erase (size_t vert_id, bool side) {
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

void GainContainer::dump () {
    cout << "Left" << endl;
    for (const auto& item : left) {
        cout << item.first << ": ";
        for (const auto& vert : item.second) {
            cout << vert << " ";
        }
        cout << endl;
    }
    cout << "Right" << endl;
    for (const auto& item : right) {
        cout << item.first << ": ";
        for (const auto& vert : item.second) {
            cout << vert << " ";
        }
        cout << endl;
    }
}