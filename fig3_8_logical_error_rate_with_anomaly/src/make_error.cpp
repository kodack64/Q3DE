// Copyright 2022 NTT CORPORATION

#include "common.hpp"


/*
Generate errors with randomly placed anomalous region
 we do not incur anomalous region when anomalous_size = 0

Coordinate of errors

- error horizontal

0 1 2 3 4
-.-.-.-.- 0
 | | | |
-.-.-.-.- 1
 | | | |
-.-.-.-.- 2
 | | | |
-.-.-.-.- 3
 | | | |
-.-.-.-.- 4

| error vertical

 0 1 2 3
          0
-.-.-.-.-
 | | | |  1
-.-.-.-.-
 | | | |  2
-.-.-.-.-
 | | | |  3
-.-.-.-.-
 | | | |  4
-.-.-.-.-
          5

. error measurement

 0 1 2 3
-.-.-.-.- 0
 | | | |
-.-.-.-.- 1
 | | | |
-.-.-.-.- 2
 | | | |
-.-.-.-.- 3
 | | | |
-.-.-.-.- 4
*/
bool make_error(
    int seed, int d,
    AnomalyInfo &anomaly_info,

    vector<tuple<int, int, int>> &syndrome_active_list,
    vector<vector<vector<bool>>> &syndrome_map,
    double error_prob,
    double error_prob_anomaly) {

    mt19937 mt(seed);
    uniform_real_distribution<> rnd(0.0, 1.0);


    // measure error (before xor)(d*(d-1))
    vector<vector<bool>> meas_value_temp(d, vector<bool>(d - 1, false));

    // measure error (after xor)(d*(d-1))
    vector<vector<bool>> meas_value_xor(d, vector<bool>(d-1, false));

    // horizontal qubit ((d+1)*(d-1))  y\in {0,n} is dummy.
    vector<vector<bool>> error_horizontal(d+1, vector<bool>(d - 1, false));

    // vertical qubit (d*d)
    vector<vector<bool>> error_vertical(d, vector<bool>(d, false));


    anomaly_info.anomaly_y = mt() % (d - anomaly_info.anomaly_size);
    anomaly_info.anomaly_x = mt() % (d - anomaly_info.anomaly_size - 1);

    rep(z, d) rep(y, d) rep(x, d - 1) syndrome_map[z][y][x] = false;
    syndrome_active_list.clear();

    // iterate cycle
    rep(z, d) {
        // refresh measurement
        rep(y, d) {
            rep(x, d - 1) {
                meas_value_xor[y][x] = meas_value_temp[y][x];
                meas_value_temp[y][x] = false;
            }
        }
        // horizontal qubit error
        rep(y, d - 1) {
            rep(x, d - 1) {
                bool is_anomalous = (anomaly_info.anomaly_y <= y) && (y < (anomaly_info.anomaly_y + anomaly_info.anomaly_size)) && (anomaly_info.anomaly_x <= x) && (x <= (anomaly_info.anomaly_x + anomaly_info.anomaly_size));
                is_anomalous = is_anomalous & (anomaly_info.anomaly_size > 0);
                double random_value = rnd(mt);
                double current_error_prob = is_anomalous ? error_prob_anomaly : error_prob;
                if (random_value < current_error_prob) {
                    error_horizontal[y + 1][x] = (!error_horizontal[y + 1][x]);
                    // cout << "| " << z << " " << y + 1 << " " << x << endl;
                }
            }
        }
        // vertical qubit error
        rep(y, d) {
            rep(x, d) {
                bool is_anomalous = (anomaly_info.anomaly_y <= y) && (y <= (anomaly_info.anomaly_y + anomaly_info.anomaly_size)) && (anomaly_info.anomaly_x <= (x - 1)) && ((x - 1) < (anomaly_info.anomaly_x + anomaly_info.anomaly_size));
                is_anomalous = is_anomalous & (anomaly_info.anomaly_size > 0);
                double random_value = rnd(mt);
                double current_error_prob = is_anomalous ? error_prob_anomaly : error_prob;
                if (random_value < current_error_prob) {
                    error_vertical[y][x] = (!error_vertical[y][x]);
                    // cout << "- " << z << " " << y << " " << x << endl;
                }
            }
        }

        // measurement with error
        rep(y, d) {
            rep(x, d - 1) {
                // gather parity
                bool parity = false;
                if (error_horizontal[y][x])
                    parity = (!parity);
                if (error_horizontal[y + 1][x])
                    parity = (!parity);
                if (error_vertical[y][x])
                    parity = (!parity);
                if (error_vertical[y][x + 1])
                    parity = (!parity);

                // if not the last cycle, consider measurement error
                if (z < (d - 1)) {
                    bool is_anomalous = (anomaly_info.anomaly_y <= y) && (y <= (anomaly_info.anomaly_y + anomaly_info.anomaly_size)) && (anomaly_info.anomaly_x <= x) && (x <= (anomaly_info.anomaly_x + anomaly_info.anomaly_size));
                    is_anomalous = is_anomalous & (anomaly_info.anomaly_size > 0);
                    double random_value = rnd(mt);
                    double current_error_prob = is_anomalous ? error_prob_anomaly : error_prob;
                    if (random_value < current_error_prob) {
                        parity = (!parity);
                        // cout << ". " << z << " " << y << " " << x << endl;
                    }
                }

                // process error
                if (parity) {
                    meas_value_xor[y][x] = (!meas_value_xor[y][x]);
                    meas_value_temp[y][x] = (!meas_value_temp[y][x]);
                }
                if (meas_value_xor[y][x]) {
                    syndrome_active_list.push_back({z, y, x});
                    syndrome_map[z][y][x] = true;
                }
            }
        }
    }

    // calculate left-boundary parity
    bool error_parity = false;
    rep(y, d) if (error_vertical[y][0]) error_parity = (!error_parity);

    return error_parity;
}