// Copyright 2022 NTT CORPORATION

#include "decoder.h"
#include "benchmark/tbench.h"
#include <random>
using namespace std;

int main() {
    // parameters
	// (default working directory is "<project_dir>/solution1/csim/build")
    string path = "../../../../../../80-Q3DE/benchmark/graph_idling/";
    int32_t d = CODESIZE; // Set codesize at decoder.h
    int32_t c = 9;
    uint32_t seed = 0;
    double error_prob = 0.01;
    int32_t sample_num = 1000;

    // Put anomaly position (x1,x2,y1,y2,z1,z2,CODESIZE);
    AnomalyInfo anomaly_info(X_L, X_R, 2, 4, 1, 3, CODESIZE);
    //AnomalyInfo anomaly_info(0, 0, 0, 0, 0, 0, CODESIZE);

    // load lattice
    LatticeInfo lattice_info(d, c, StabType::STAB_X);
    load_lattice(path, lattice_info);
    //visualize_lattice(lattice_info);

    // create error info
    ErrorInfo error_info(lattice_info);

    mt19937 mt;
    mt.seed(seed);
    int32_t fail_count = 0;
    for (int sample_count = 0; sample_count < sample_num; ++sample_count) {
        // sample error pattern
        uint32_t error_seed = mt();
    	std::cout << "*************************** Sample index: " << sample_count << " ***************************"<< std::endl;
        sample(lattice_info, error_info, error_prob, error_seed);
        //visualize_error(lattice_info, error_info);

        // try matching
        auto nodes = extract_raw_nodes(lattice_info, error_info);

#ifndef ANOMALY
        auto pairs_trial = match_trial(nodes);
        //auto pairs_test = match_greedy(nodes);
        auto pairs_test = match_iterative_greedy(nodes);
#else
        auto pairs_trial = match_trial_with_anomaly(nodes, anomaly_info);
        //auto pairs_trial = match_greedy_with_anomaly(nodes, anomaly_info);
        auto pairs_test = match_iterative_greedy_with_anomaly(nodes, anomaly_info);
#endif

        // check results
        bool result = compare(pairs_trial, pairs_test);

        if (!result) {
            visualize_match(lattice_info, error_info, nodes, pairs_trial, pairs_test);
            fail_count += 1;
        }
    }
    std::cout << "Mismatch " << fail_count << " / " << sample_num << std::endl;
    std::cout.flush();
    if(fail_count > 0){
    	std::cerr << "Invalid cases found" << endl;
        exit(-1);
    }
    return 0;
}
