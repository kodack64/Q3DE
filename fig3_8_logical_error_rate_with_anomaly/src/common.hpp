// Copyright 2022 NTT CORPORATION

#pragma once

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <tuple>
#include <vector>
#include <stdexcept>

#include "blossom5/PerfectMatching.h"

#define rep(i, n) for (int i = 0; i < n; ++i)
#define rep2(i, k, n) for (int i = k; i < n; ++i)

using namespace std;

class AnomalyInfo {
   public:
    const int anomaly_size;
    // [0,d-1]*[anomaly_y,anomaly_y+anomaly_size]*[anomaly_x,anomaly_x+anomaly_size]=high error rate area
    int anomaly_x;
    int anomaly_y;
    AnomalyInfo(int _anomaly_size) : anomaly_size(_anomaly_size){};
};

class RecoveryInfo {
 public:
    vector<vector<vector<bool>>> recovery_meas;        // measure correction ((d+1)*d*(d-1))  x\in {0,n} is dummy.
    vector<vector<vector<bool>>> recovery_vertical;  // horizontal correction (d*(d+1)*(d-1))  y\in {0,n} is dummy.
    vector<vector<vector<bool>>> recovery_horizontal;    // vertical correction (d*d*d)
    RecoveryInfo(int d) {
        vector<bool> for_init;
        vector<vector<bool>> for_init2;
        rep(k, d - 1) for_init.push_back(false);
        rep(j, d) for_init2.push_back(for_init);
        rep(i, d + 1) recovery_meas.push_back(for_init2);  //(d+1)*d*(d-1)
        for_init.clear();
        for_init2.clear();

        rep(k, d - 1) for_init.push_back(false);
        rep(j, d + 1) for_init2.push_back(for_init);
        rep(i, d) recovery_vertical.push_back(for_init2);  // d*(d+1)*(d-1)
        for_init.clear();
        for_init2.clear();

        rep(k, d) for_init.push_back(false);
        rep(j, d) for_init2.push_back(for_init);
        rep(i, d) recovery_horizontal.push_back(for_init2);  // d*d*d
        for_init.clear();
        for_init2.clear();
    }
};

bool make_error(
    int seed, int d,
    AnomalyInfo& anomaly_info,
    vector<tuple<int, int, int>>& syndrome_active_list,
    vector<vector<vector<bool>>>& syndrome_map,
    double error_prob,
    double error_prob_anomaly);

void correction_uniform(
    int d,
    RecoveryInfo& recovery_info,
    const vector<tuple<int, int, int>>& syndrome_active_list);

void correction_weighted(
    int d,
    RecoveryInfo& recovery_info,
    const vector<tuple<int, int, int>>& syndrome_active_list,
    const AnomalyInfo& anomaly_info);

void visualize(
    int d,
    const RecoveryInfo& recovery_info,
    const vector<vector<vector<bool>>>& syndrome_map);