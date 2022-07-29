// Copyright 2022 NTT CORPORATION

#include "common.hpp"

// perform matching with considering the position of anomalous region
//  the estimated errors are stored in recovery info
void correction_weighted(
    int d,
    RecoveryInfo &recovery_info,
    const vector<tuple<int, int, int>> &syndrome_active_list,
    const AnomalyInfo& anomaly_info) {
    

    // we assume cost of normal edge is 1 and anomalous edge is 0
    const int cost_normal = 1;
    const int cost_anomaly = 0;

    const int anomaly_y = anomaly_info.anomaly_y;
    const int anomaly_x = anomaly_info.anomaly_x;
    const int anomaly_size = anomaly_info.anomaly_size;

    int cur_z, cur_y, cur_x;
    int cost_candidate[3];

    rep(i, d + 1) rep(j, d) rep(k, d - 1) recovery_info.recovery_meas[i][j][k] = false;
    rep(i, d) rep(j, d + 1) rep(k, d - 1) recovery_info.recovery_vertical[i][j][k] = false;
    rep(i, d) rep(j, d) rep(k, d) recovery_info.recovery_horizontal[i][j][k] = false;

    if (syndrome_active_list.size() == 0)
        return;

    // distance to border
    vector<int> cost_boundary;
    // nearest point of anomalous region
    vector<tuple<int, int, int>> near;
    rep(i, syndrome_active_list.size()) {

        // pick nearest point of anomalous region
        cur_z = get<0>(syndrome_active_list[i]);
        cur_y = get<1>(syndrome_active_list[i]);
        cur_x = get<2>(syndrome_active_list[i]);
        if (cur_y < anomaly_y)
            cur_y = anomaly_y;
        if (cur_y > (anomaly_y + anomaly_size))
            cur_y = anomaly_y + anomaly_size;
        if (cur_x < anomaly_x)
            cur_x = anomaly_x;
        if (cur_x > (anomaly_x + anomaly_size))
            cur_x = anomaly_x + anomaly_size;

        near.push_back({cur_z, cur_y, cur_x});

        // node to near boundary
        cost_candidate[0] = cost_normal * min(get<2>(syndrome_active_list[i]) + 1, d - get<2>(syndrome_active_list[i]) - 1);

        // (node to anomaly) + (anomaly to near boundary)
        cost_candidate[1] = (
            cost_normal * abs(cur_y - get<1>(syndrome_active_list[i]))) + (cost_normal * abs(cur_x - get<2>(syndrome_active_list[i])))
            + 
            min(
                (cost_anomaly * (cur_x - anomaly_x)) + (cost_normal * (anomaly_x + 1)), 
                (cost_anomaly * (anomaly_x + anomaly_size - cur_x)) + (cost_normal * (d - (anomaly_x + anomaly_size) - 1))
            );

        cost_boundary.push_back(min(cost_candidate[0], cost_candidate[1]));
    }

    // set weight
    bool odd = (syndrome_active_list.size() & 1);
    int num_pair = ((int)syndrome_active_list.size() + 1) / 2;
    PerfectMatching *pm = new PerfectMatching(2 * num_pair, num_pair * (2 * num_pair - 1));
    rep(i, syndrome_active_list.size()) {
        // cout << "vector " << i << " : " << syndrome_active_list[i].first << " " << syndrome_active_list[i].second << endl;
        rep(j, i) {

            // manhattan distance
            cost_candidate[0] = 
                (cost_normal * abs(get<0>(syndrome_active_list[i]) - get<0>(syndrome_active_list[j]))) + 
                (cost_normal * abs(get<1>(syndrome_active_list[i]) - get<1>(syndrome_active_list[j]))) + 
                (cost_normal * abs(get<2>(syndrome_active_list[i]) - get<2>(syndrome_active_list[j])));

            // (node to anomalous region) + (move inside anomalous region) + (anomalous region to node)
            cost_candidate[1] = 
                (cost_normal * abs(get<0>(syndrome_active_list[i]) - get<0>(near[i]))) + 
                (cost_normal * abs(get<1>(syndrome_active_list[i]) - get<1>(near[i]))) + 
                (cost_normal * abs(get<2>(syndrome_active_list[i]) - get<2>(near[i])));
            cost_candidate[1] += 
                (cost_anomaly * abs(get<0>(near[i]) - get<0>(near[j]))) + 
                (cost_anomaly * abs(get<1>(near[i]) - get<1>(near[j]))) + 
                (cost_anomaly * abs(get<2>(near[i]) - get<2>(near[j])));
            cost_candidate[1] += 
                (cost_normal * abs(get<0>(syndrome_active_list[j]) - get<0>(near[j]))) + 
                (cost_normal * abs(get<1>(syndrome_active_list[j]) - get<1>(near[j]))) + 
                (cost_normal * abs(get<2>(syndrome_active_list[j]) - get<2>(near[j])));

            // (node1 to boundary) + (node2 to boundary)
            cost_candidate[2] = cost_boundary[i] + cost_boundary[j];

            int minimum_cost = min(min(cost_candidate[0], cost_candidate[1]), cost_candidate[2]);
            pm->AddEdge(j, i, minimum_cost);
        }
        if (odd)
            pm->AddEdge(i, 2 * num_pair - 1, cost_boundary[i]);
    }

    // solve matching
    pm->options.verbose = false;
    pm->Solve();

    rep(l, 2 * num_pair) {
        int m = pm->GetMatch(l);
        if (!(l < m)) continue;
        // matched to boundary
        if (odd && (m == (2 * num_pair) - 1)) {
            cost_candidate[0] = cost_normal * min(get<2>(syndrome_active_list[l]) + 1, d - get<2>(syndrome_active_list[l]) - 1);
            cost_candidate[1] = (cost_normal * abs(get<0>(near[l]) - get<0>(syndrome_active_list[l]))) + (cost_normal * abs(get<1>(near[l]) - get<1>(syndrome_active_list[l]))) + (cost_normal * abs(get<2>(near[l]) - get<2>(syndrome_active_list[l]))) + min((cost_anomaly * (get<2>(near[l]) - anomaly_x)) + (cost_normal * (anomaly_x + 1)), (cost_anomaly * (anomaly_x + anomaly_size - get<2>(near[l]))) + (cost_normal * (d - (anomaly_x + anomaly_size) - 1)));
            if (cost_candidate[0] <= cost_candidate[1]) {
                if ((get<2>(syndrome_active_list[l]) + 1) < (d - get<2>(syndrome_active_list[l]) - 1)) {
                    for (int k = 0; k < (get<2>(syndrome_active_list[l]) + 1); k++)
                        recovery_info.recovery_horizontal[get<0>(syndrome_active_list[l])][get<1>(syndrome_active_list[l])][k] = (!recovery_info.recovery_horizontal[get<0>(syndrome_active_list[l])][get<1>(syndrome_active_list[l])][k]);
                } else {
                    for (int k = get<2>(syndrome_active_list[l]) + 1; k < d; k++)
                        recovery_info.recovery_horizontal[get<0>(syndrome_active_list[l])][get<1>(syndrome_active_list[l])][k] = (!recovery_info.recovery_horizontal[get<0>(syndrome_active_list[l])][get<1>(syndrome_active_list[l])][k]);
                }
            } else {
                cur_z = get<0>(syndrome_active_list[l]);
                cur_y = get<1>(syndrome_active_list[l]);
                cur_x = get<2>(syndrome_active_list[l]);
                while (cur_z != get<0>(near[l])) {
                    if (cur_z < get<0>(near[l])) {
                        recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x]);
                        cur_z++;
                    } else {
                        recovery_info.recovery_meas[cur_z][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z][cur_y][cur_x]);
                        cur_z--;
                    }
                }
                while (cur_y != get<1>(near[l])) {
                    if (cur_y < get<1>(near[l])) {
                        recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x]);
                        cur_y++;
                    } else {
                        recovery_info.recovery_vertical[cur_z][cur_y][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y][cur_x]);
                        cur_y--;
                    }
                }
                while (cur_x != get<2>(near[l])) {
                    if (cur_x < get<2>(near[l])) {
                        recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1]);
                        cur_x++;
                    } else {
                        recovery_info.recovery_horizontal[cur_z][cur_y][cur_x] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x]);
                        cur_x--;
                    }
                }
                if (((cost_anomaly * (get<2>(near[l]) - anomaly_x)) + (cost_normal * (anomaly_x + 1))) < ((cost_anomaly * (anomaly_x + anomaly_size - get<2>(near[l]))) + (cost_normal * (d - (anomaly_x + anomaly_size) - 1)))) {
                    for (int k = 0; k < cur_x + 1; k++)
                        recovery_info.recovery_horizontal[cur_z][cur_y][k] = (!recovery_info.recovery_horizontal[cur_z][cur_y][k]);
                } else {
                    for (int k = cur_x + 1; k < d; k++)
                        recovery_info.recovery_horizontal[cur_z][cur_y][k] = (!recovery_info.recovery_horizontal[cur_z][cur_y][k]);
                }
            }
        }

        // matched between nodes
        else {
            /// cost_candidate i|>s
            cost_candidate[0] = (cost_normal * abs(get<0>(syndrome_active_list[l]) - get<0>(syndrome_active_list[m]))) + (cost_normal * abs(get<1>(syndrome_active_list[l]) - get<1>(syndrome_active_list[m]))) + (cost_normal * abs(get<2>(syndrome_active_list[l]) - get<2>(syndrome_active_list[m])));
            cost_candidate[1] = (cost_normal * abs(get<0>(syndrome_active_list[l]) - get<0>(near[l]))) + (cost_normal * abs(get<1>(syndrome_active_list[l]) - get<1>(near[l]))) + (cost_normal * abs(get<2>(syndrome_active_list[l]) - get<2>(near[l])));
            cost_candidate[1] += (cost_anomaly * abs(get<0>(near[l]) - get<0>(near[m]))) + (cost_anomaly * abs(get<1>(near[l]) - get<1>(near[m]))) + (cost_anomaly * abs(get<2>(near[l]) - get<2>(near[m])));
            cost_candidate[1] += (cost_normal * abs(get<0>(syndrome_active_list[m]) - get<0>(near[m]))) + (cost_normal * abs(get<1>(syndrome_active_list[m]) - get<1>(near[m]))) + (cost_normal * abs(get<2>(syndrome_active_list[m]) - get<2>(near[m])));
            cost_candidate[2] = cost_boundary[l] + cost_boundary[m];


            // manhattan path
            if ((cost_candidate[0] <= cost_candidate[1]) && (cost_candidate[0] <= cost_candidate[2])) {
                cur_z = get<0>(syndrome_active_list[l]);
                cur_y = get<1>(syndrome_active_list[l]);
                cur_x = get<2>(syndrome_active_list[l]);
                while (cur_z != get<0>(syndrome_active_list[m])) {
                    if (cur_z < get<0>(syndrome_active_list[m])) {
                        recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x]);
                        cur_z++;
                    } else {
                        recovery_info.recovery_meas[cur_z][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z][cur_y][cur_x]);
                        cur_z--;
                    }
                }
                while (cur_y != get<1>(syndrome_active_list[m])) {
                    if (cur_y < get<1>(syndrome_active_list[m])) {
                        recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x]);
                        cur_y++;
                    } else {
                        recovery_info.recovery_vertical[cur_z][cur_y][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y][cur_x]);
                        cur_y--;
                    }
                }
                while (cur_x != get<2>(syndrome_active_list[m])) {
                    if (cur_x < get<2>(syndrome_active_list[m])) {
                        recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1]);
                        cur_x++;
                    } else {
                        recovery_info.recovery_horizontal[cur_z][cur_y][cur_x] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x]);
                        cur_x--;
                    }
                }
            }

            // manhattan via anomalous region
            else if ((cost_candidate[1] <= cost_candidate[0]) && (cost_candidate[1] <= cost_candidate[2])) {
                // syndrome_active_list[l] -> near[l] -> near[m] -> syndrome_active_list[m]
                cur_z = get<0>(syndrome_active_list[l]);
                cur_y = get<1>(syndrome_active_list[l]);
                cur_x = get<2>(syndrome_active_list[l]);

                // move from node1 to anomalous region
                while (cur_z != get<0>(near[l])) {
                    if (cur_z < get<0>(near[l])) {
                        recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x]);
                        cur_z++;
                    } else {
                        recovery_info.recovery_meas[cur_z][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z][cur_y][cur_x]);
                        cur_z--;
                    }
                }
                while (cur_y != get<1>(near[l])) {
                    if (cur_y < get<1>(near[l])) {
                        recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x]);
                        cur_y++;
                    } else {
                        recovery_info.recovery_vertical[cur_z][cur_y][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y][cur_x]);
                        cur_y--;
                    }
                }
                while (cur_x != get<2>(near[l])) {
                    if (cur_x < get<2>(near[l])) {
                        recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1]);
                        cur_x++;
                    } else {
                        recovery_info.recovery_horizontal[cur_z][cur_y][cur_x] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x]);
                        cur_x--;
                    }
                }

                // move inside anomalous region
                while (cur_z != get<0>(near[m])) {
                    if (cur_z < get<0>(near[m])) {
                        recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x]);
                        cur_z++;
                    } else {
                        recovery_info.recovery_meas[cur_z][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z][cur_y][cur_x]);
                        cur_z--;
                    }
                }
                while (cur_y != get<1>(near[m])) {
                    if (cur_y < get<1>(near[m])) {
                        recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x]);
                        cur_y++;
                    } else {
                        recovery_info.recovery_vertical[cur_z][cur_y][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y][cur_x]);
                        cur_y--;
                    }
                }
                while (cur_x != get<2>(near[m])) {
                    if (cur_x < get<2>(near[m])) {
                        recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1]);
                        cur_x++;
                    } else {
                        recovery_info.recovery_horizontal[cur_z][cur_y][cur_x] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x]);
                        cur_x--;
                    }
                }

                // move from anomalous region to node2
                while (cur_z != get<0>(syndrome_active_list[m])) {
                    if (cur_z < get<0>(syndrome_active_list[m])) {
                        recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x]);
                        cur_z++;
                    } else {
                        recovery_info.recovery_meas[cur_z][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z][cur_y][cur_x]);
                        cur_z--;
                    }
                }
                while (cur_y != get<1>(syndrome_active_list[m])) {
                    if (cur_y < get<1>(syndrome_active_list[m])) {
                        recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x]);
                        cur_y++;
                    } else {
                        recovery_info.recovery_vertical[cur_z][cur_y][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y][cur_x]);
                        cur_y--;
                    }
                }
                while (cur_x != get<2>(syndrome_active_list[m])) {
                    if (cur_x < get<2>(syndrome_active_list[m])) {
                        recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1]);
                        cur_x++;
                    } else {
                        recovery_info.recovery_horizontal[cur_z][cur_y][cur_x] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x]);
                        cur_x--;
                    }
                }
            }

            // two nodes are matched to boundary
            else {
                // node0
                cost_candidate[0] = cost_normal * min(get<2>(syndrome_active_list[l]) + 1, d - get<2>(syndrome_active_list[l]) - 1);
                cost_candidate[1] = (cost_normal * abs(get<0>(near[l]) - get<0>(syndrome_active_list[l]))) + (cost_normal * abs(get<1>(near[l]) - get<1>(syndrome_active_list[l]))) + (cost_normal * abs(get<2>(near[l]) - get<2>(syndrome_active_list[l]))) + min((cost_anomaly * (get<2>(near[l]) - anomaly_x)) + (cost_normal * (anomaly_x + 1)), (cost_anomaly * (anomaly_x + anomaly_size - get<2>(near[l]))) + (cost_normal * (d - (anomaly_x + anomaly_size) - 1)));
                // node0 to boundary directly
                if (cost_candidate[0] <= cost_candidate[1]) {
                    if ((get<2>(syndrome_active_list[l]) + 1) < (d - get<2>(syndrome_active_list[l]) - 1)) {
                        for (int k = 0; k < (get<2>(syndrome_active_list[l]) + 1); k++)
                            recovery_info.recovery_horizontal[get<0>(syndrome_active_list[l])][get<1>(syndrome_active_list[l])][k] = (!recovery_info.recovery_horizontal[get<0>(syndrome_active_list[l])][get<1>(syndrome_active_list[l])][k]);
                    } else {
                        for (int k = get<2>(syndrome_active_list[l]) + 1; k < d; k++)
                            recovery_info.recovery_horizontal[get<0>(syndrome_active_list[l])][get<1>(syndrome_active_list[l])][k] = (!recovery_info.recovery_horizontal[get<0>(syndrome_active_list[l])][get<1>(syndrome_active_list[l])][k]);
                    }
                }
                // node0 to boundary via anomalous region
                else {
                    cur_z = get<0>(syndrome_active_list[l]);
                    cur_y = get<1>(syndrome_active_list[l]);
                    cur_x = get<2>(syndrome_active_list[l]);
                    while (cur_z != get<0>(near[l])) {
                        if (cur_z < get<0>(near[l])) {
                            recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x]);
                            cur_z++;
                        } else {
                            recovery_info.recovery_meas[cur_z][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z][cur_y][cur_x]);
                            cur_z--;
                        }
                    }
                    while (cur_y != get<1>(near[l])) {
                        if (cur_y < get<1>(near[l])) {
                            recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x]);
                            cur_y++;
                        } else {
                            recovery_info.recovery_vertical[cur_z][cur_y][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y][cur_x]);
                            cur_y--;
                        }
                    }
                    while (cur_x != get<2>(near[l])) {
                        if (cur_x < get<2>(near[l])) {
                            recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1]);
                            cur_x++;
                        } else {
                            recovery_info.recovery_horizontal[cur_z][cur_y][cur_x] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x]);
                            cur_x--;
                        }
                    }
                    if (((cost_anomaly * (get<2>(near[l]) - anomaly_x)) + (cost_normal * (anomaly_x + 1))) < ((cost_anomaly * (anomaly_x + anomaly_size - get<2>(near[l]))) + (cost_normal * (d - (anomaly_x + anomaly_size) - 1)))) {
                        for (int k = 0; k < cur_x + 1; k++)
                            recovery_info.recovery_horizontal[cur_z][cur_y][k] = (!recovery_info.recovery_horizontal[cur_z][cur_y][k]);
                    } else {
                        for (int k = cur_x + 1; k < d; k++)
                            recovery_info.recovery_horizontal[cur_z][cur_y][k] = (!recovery_info.recovery_horizontal[cur_z][cur_y][k]);
                    }
                }

                // node1
                cost_candidate[0] = cost_normal * min(get<2>(syndrome_active_list[m]) + 1, d - get<2>(syndrome_active_list[m]) - 1);
                cost_candidate[1] = (cost_normal * abs(get<0>(near[m]) - get<0>(syndrome_active_list[m]))) + (cost_normal * abs(get<1>(near[m]) - get<1>(syndrome_active_list[m]))) + (cost_normal * abs(get<2>(near[m]) - get<2>(syndrome_active_list[m]))) + min((cost_anomaly * (get<2>(near[m]) - anomaly_x)) + (cost_normal * (anomaly_x + 1)), (cost_anomaly * (anomaly_x + anomaly_size - get<2>(near[m]))) + (cost_normal * (d - (anomaly_x + anomaly_size) - 1)));
                // node1 to boundary directly
                if (cost_candidate[0] <= cost_candidate[1]) {
                    if ((get<2>(syndrome_active_list[m]) + 1) < (d - get<2>(syndrome_active_list[m]) - 1)) {
                        for (int k = 0; k < (get<2>(syndrome_active_list[m]) + 1); k++)
                            recovery_info.recovery_horizontal[get<0>(syndrome_active_list[m])][get<1>(syndrome_active_list[m])][k] = (!recovery_info.recovery_horizontal[get<0>(syndrome_active_list[m])][get<1>(syndrome_active_list[m])][k]);
                    } else {
                        for (int k = get<2>(syndrome_active_list[m]) + 1; k < d; k++)
                            recovery_info.recovery_horizontal[get<0>(syndrome_active_list[m])][get<1>(syndrome_active_list[m])][k] = (!recovery_info.recovery_horizontal[get<0>(syndrome_active_list[m])][get<1>(syndrome_active_list[m])][k]);
                    }
                }
                // node1 to boundary via anomalous region
                else {
                    cur_z = get<0>(syndrome_active_list[m]);
                    cur_y = get<1>(syndrome_active_list[m]);
                    cur_x = get<2>(syndrome_active_list[m]);
                    while (cur_z != get<0>(near[m])) {
                        if (cur_z < get<0>(near[m])) {
                            recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z + 1][cur_y][cur_x]);
                            cur_z++;
                        } else {
                            recovery_info.recovery_meas[cur_z][cur_y][cur_x] = (!recovery_info.recovery_meas[cur_z][cur_y][cur_x]);
                            cur_z--;
                        }
                    }
                    while (cur_y != get<1>(near[m])) {
                        if (cur_y < get<1>(near[m])) {
                            recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y + 1][cur_x]);
                            cur_y++;
                        } else {
                            recovery_info.recovery_vertical[cur_z][cur_y][cur_x] = (!recovery_info.recovery_vertical[cur_z][cur_y][cur_x]);
                            cur_y--;
                        }
                    }
                    while (cur_x != get<2>(near[m])) {
                        if (cur_x < get<2>(near[m])) {
                            recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x + 1]);
                            cur_x++;
                        } else {
                            recovery_info.recovery_horizontal[cur_z][cur_y][cur_x] = (!recovery_info.recovery_horizontal[cur_z][cur_y][cur_x]);
                            cur_x--;
                        }
                    }
                    if (((cost_anomaly * (get<2>(near[m]) - anomaly_x)) + (cost_normal * (anomaly_x + 1))) < ((cost_anomaly * (anomaly_x + anomaly_size - get<2>(near[m]))) + (cost_normal * (d - (anomaly_x + anomaly_size) - 1)))) {
                        for (int k = 0; k < cur_x + 1; k++)
                            recovery_info.recovery_horizontal[cur_z][cur_y][k] = (!recovery_info.recovery_horizontal[cur_z][cur_y][k]);
                    } else {
                        for (int k = cur_x + 1; k < d; k++)
                            recovery_info.recovery_horizontal[cur_z][cur_y][k] = (!recovery_info.recovery_horizontal[cur_z][cur_y][k]);
                    }
                }
            }
        }
    }
    delete pm;
    return;
}
