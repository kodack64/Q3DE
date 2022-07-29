// Copyright 2022 NTT CORPORATION

#include "common.hpp"


// calculate left-boundary parity of estimated errors
bool check(int d, const RecoveryInfo &recovery_info) {
    bool recovery_parity = false;
    rep(z, d) {
        rep(y, d) {
            if (recovery_info.recovery_horizontal[z][y][0])
                recovery_parity = (!recovery_parity);
        }
    }
    return recovery_parity;
}

void invalid_exit(string msg) {
    cerr << msg << endl;
    throw std::runtime_error(msg.c_str());
    exit(1);
}

int main(int argc, char *argv[]) {
    // distance
    int d = 11;
    // anomaly size
    int anomaly_size = 0;
    // consider weight or not
    bool use_weight = false;
    // trial count
    int trial_count = 10000;
    // physical error probability
    double error_prob = 1e-2;
    // physical error probability of anomalous qubit
    double error_prob_anomaly = 0.5;

    // fix seed when executed without argument
    int seed = 42;
    bool visualize_flag = true;

    if (argc > 1) {

        if (argc == 6) {
            // parse argument
            d = atoi(argv[1]);
            anomaly_size = atoi(argv[2]);
            use_weight = (atoi(argv[3]) == 1);
            trial_count = atoi(argv[4]);
            error_prob = atof(argv[5]);

        } else {
            invalid_exit("invalid argument count");
        }

        error_prob_anomaly = 0.5;

        // use random seed when executed with argument
        random_device rd;
        seed = rd();
        visualize_flag = false;
    }
    if (d % 2 == 0) {
        invalid_exit("code distance is even");
    }
    if (d <= anomaly_size + 1) {
        invalid_exit("anomaly_size is too large");
    }
    if (anomaly_size == 0 && use_weight) {
        invalid_exit("no anomaly but choose weighted decoding");
    }

    AnomalyInfo anomaly_info(anomaly_size);
    RecoveryInfo recovery_info(d);

    mt19937 mt(seed);
    

    // measure error (after xor)(d*d*(d-1))
    vector<vector<vector<bool>>> syndrome_map(d, vector<vector<bool>>(d, vector<bool>(d-1, false)));
    // the result of measure (d*d*(d-1))
    vector<tuple<int, int, int>> syndrome_active_list;

    // perform trials
    clock_t start = clock();
    int misscount = 0;
    rep(_, trial_count) {

        int error_seed = mt();

        bool error_parity = make_error(error_seed, d, anomaly_info, syndrome_active_list, syndrome_map, error_prob, error_prob_anomaly);

        if (use_weight)
            correction_weighted(d, recovery_info, syndrome_active_list, anomaly_info);
        else
            correction_uniform(d, recovery_info, syndrome_active_list);

        bool recovery_parity = check(d, recovery_info);

        if (recovery_parity != error_parity)
            misscount++;

        if ((recovery_parity != error_parity) && visualize_flag)
            visualize(d, recovery_info, syndrome_map);
    }
    double logical_error_rate = ((double)misscount) / ((double)trial_count);
    clock_t end = clock();

    // output to file
    stringstream ss;
    ss << "result_" << d << "_" << anomaly_size << "_" << use_weight << "_" << error_prob << ".txt";
    fstream ofs(ss.str(), ios::app);
    ofs << trial_count << " " << logical_error_rate << endl;
    cout << trial_count << " " << logical_error_rate << endl;
    ofs.close();

    return 0;
}
