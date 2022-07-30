// Copyright 2022 NTT CORPORATION

#include "tbench.h"
#include <random>
#include <iostream>
using namespace std;

//#define ENABLE_ANOMALY

int main() {
    // params
    string path = "../graph_idling/";
    int32_t d = 5;
    int32_t c = 5;
    uint32_t seed = 0;
    double error_prob = 0.01;
    int32_t sample_num = 10;

    // load lattice
    LatticeInfo lattice_info(d, c, StabType::STAB_X);
    load_lattice(path, lattice_info);
    //visualize_lattice(lattice_info);

    // create error info
    ErrorInfo error_info(lattice_info);

    mt19937 mt;
    mt.seed(seed);

#ifdef ENABLE_ANOMALY
    AnomalyInfo anomaly_info(2, 4, 0, 5, 0, 5, d);
#endif

    int32_t fail_count = 0;
    for (int sample_count = 0; sample_count < sample_num; ++sample_count) {
        // sample error pattern
        uint32_t error_seed = mt();
        sample(lattice_info, error_info, error_prob, error_seed);
        //visualize_error(lattice_info, error_info);

        // try matching
        auto nodes = extract_raw_nodes(lattice_info, error_info);
#ifndef ENABLE_ANOMALY
        auto pairs_trial = match_trial(nodes);
        auto pairs_test = match_greedy(nodes);
#else
        auto pairs_trial = match_trial_with_anomaly(nodes, anomaly_info);
        auto pairs_test = match_greedy_with_anomaly(nodes, anomaly_info);
#endif

        // check results
        bool result = compare(pairs_trial, pairs_test);
        if (!result) fail_count += 1;

        if (result) {
            visualize_match(lattice_info, error_info, nodes, pairs_trial, pairs_test);
        }
    }
    cout.flush();
    if (fail_count > 0) {
        exit(-1);
    }
    return 0;
}