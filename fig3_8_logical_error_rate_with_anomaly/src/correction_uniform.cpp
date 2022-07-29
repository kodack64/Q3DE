// Copyright 2022 NTT CORPORATION

#include "common.hpp"

// perform matching without considering the position of anomalous region
//  the estimated errors are stored in recovery info
void correction_uniform(
    int d,
    RecoveryInfo &recovery_info,
    const vector<tuple<int, int, int>> &syndrome_active_list) {

    // init bitmap
    rep(i, d + 1) rep(j, d) rep(k, d - 1) recovery_info.recovery_meas[i][j][k] = false;
    rep(i, d) rep(j, d + 1) rep(k, d - 1) recovery_info.recovery_vertical[i][j][k] = false;
    rep(i, d) rep(j, d) rep(k, d) recovery_info.recovery_horizontal[i][j][k] = false;

    // return if no active node
    if (syndrome_active_list.size() == 0) return;

    // calculate cost to boundary
    vector<int> cost_boundary;
    rep(i, syndrome_active_list.size()) {
        int xi = get<2>(syndrome_active_list[i]);
        cost_boundary.push_back(min(xi + 1, d - xi - 1));
    }


    // set weight
    int num_pair = ((int)syndrome_active_list.size() + 1) / 2;
    bool odd = (syndrome_active_list.size() & 1);
    PerfectMatching *pm = new PerfectMatching(2 * num_pair, num_pair * (2 * num_pair - 1));
    rep(i, syndrome_active_list.size()) {
        // cout << "vector " << i << " : " << syndrome_active_list[i].first << " " << syndrome_active_list[i].second << endl;
        rep(j, i) {
            int zi = get<0>(syndrome_active_list[i]);
            int yi = get<1>(syndrome_active_list[i]);
            int xi = get<2>(syndrome_active_list[i]);
            int zj = get<0>(syndrome_active_list[j]);
            int yj = get<1>(syndrome_active_list[j]);
            int xj = get<2>(syndrome_active_list[j]);

            int dist = abs(zi - zj) + abs(yi - yj) + abs(xi - xj);
            pm->AddEdge(j, i, min(dist, cost_boundary[i] + cost_boundary[j]));
        }
        if (odd)
            pm->AddEdge(i, 2 * num_pair - 1, cost_boundary[i]);
    }

    // matching
    pm->options.verbose = false;
    pm->Solve();


    // reconstruct recovery paths
    rep(l, 2 * num_pair) {
        int m = pm->GetMatch(l);

        // iterate only for (l<m)
        if (!(l < m)) continue;

        int zl = get<0>(syndrome_active_list[l]);
        int yl = get<1>(syndrome_active_list[l]);
        int xl = get<2>(syndrome_active_list[l]);
        // cout << k << "-" << l << endl;

        // paired to boundary
        if (odd && (m == (2 * num_pair) - 1)) {
            if ((xl + 1) < (d - xl - 1)) {
                for (int x = 0; x < (xl + 1); x++)
                    recovery_info.recovery_horizontal[zl][yl][x] = (!recovery_info.recovery_horizontal[zl][yl][x]);
            } else {
                for (int x = (xl + 1); x < d; x++)
                    recovery_info.recovery_horizontal[zl][yl][x] = (!recovery_info.recovery_horizontal[zl][yl][x]);
            }
        }

        // paired between nodes
        else {
            int zm = get<0>(syndrome_active_list[m]);
            int ym = get<1>(syndrome_active_list[m]);
            int xm = get<2>(syndrome_active_list[m]);

            // choose manhattan path
            int dist = abs(zl - zm) + abs(yl - ym) + abs(xl - xm);
            if (dist < (cost_boundary[l] + cost_boundary[m])) {

                // move along with z,y,x
                int cur_z = zl;
                int cur_y = yl;
                int cur_x = xl;
                while (cur_z != zm) {
                    if (cur_z < zm) {
                        recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x]);
                        cur_z++;
                    } else {
                        recovery_info.recovery_meas[cur_z][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z][cur_y][cur_x]);
                        cur_z--;
                    }
                }
                while (cur_y != ym) {
                    if (cur_y < ym) {
                        recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x]);
                        cur_y++;
                    } else {
                        recovery_info.recovery_vertical[cur_z][cur_y][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y][cur_x]);
                        cur_y--;
                    }
                }
                while (cur_x != xm) {
                    if (cur_x < xm) {
                        recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1]);
                        cur_x++;
                    } else {
                        recovery_info.recovery_horizontal[cur_z][cur_y][cur_x] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x]);
                        cur_x--;
                    }
                }
            }

            // choose boundary path
            else {

                // match l-node to boundary
                if ((xl + 1) < (d - xl - 1)) {
                    for (int x = 0; x < (xl + 1); x++)
                        recovery_info.recovery_horizontal[zl][yl][x] = (!recovery_info.recovery_horizontal[zl][yl][x]);
                } else {
                    for (int x = (xl + 1); x < d; x++)
                        recovery_info.recovery_horizontal[zl][yl][x] = (!recovery_info.recovery_horizontal[zl][yl][x]);
                }

                // match m-node to boundary
                if ((xm + 1) < (d - xm - 1)) {
                    for (int x = 0; x < (xm + 1); x++)
                        recovery_info.recovery_horizontal[zm][ym][x] = (!recovery_info.recovery_horizontal[zm][ym][x]);
                } else {
                    for (int x = (xm + 1); x < d; x++)
                        recovery_info.recovery_horizontal[zm][ym][x] = (!recovery_info.recovery_horizontal[zm][ym][x]);
                }
            }
        }
    }
    delete pm;
    return;
}
