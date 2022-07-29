// Copyright 2022 NTT CORPORATION

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _DEBUG
#define DEBUG_FLAG
#include <conio.h>
#endif

#ifdef _MSC_VER
#include <conio.h>
#endif
//#define DEBUG_FLAG

#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include "error_lattice.hpp"
#include "syndrome_lattice.hpp"

void set_error_lattice_uniform(double error_prob_x, double error_prob_y, double error_prob_z, ErrorLattice& error_lattice) {
	for (unsigned int z = 0; z < 2 * error_lattice._cycle; ++z) {
		for (unsigned int y = 0; y < 2 * error_lattice._distance; ++y) {
			for (unsigned int x = 0; x < 2 * error_lattice._distance; ++x) {
				Position pos(x, y, z);
				if (!error_lattice.is_exist(pos)) continue;
				NodeIndex id = error_lattice.position_to_index(pos, true);
				error_lattice.stack_error_prob_X(id, error_prob_x);
				error_lattice.stack_error_prob_Y(id, error_prob_y);
				error_lattice.stack_error_prob_Z(id, error_prob_z);
			}
		}
	}
}

void anomaly_detect_region_long(std::string filename, int distance, int cycle, uint32_t trial_count, double error_prob, double error_prob_anomaly_ratio, int anomaly_size, int anomaly_pos) {
    Random random;
#ifdef _DEBUG
    unsigned int seed = 0;
    random.set_seed(seed);
#else
    unsigned int seed = random.set_random_seed();
#endif

    ErrorLattice error_lattice(distance, cycle * 4);
    set_error_lattice_uniform(error_prob / 3, error_prob / 3, error_prob / 3, error_lattice);

    SyndromeLatticeX syndrome_lattice_x(distance, cycle * 4);
    SyndromeLatticeZ syndrome_lattice_z(distance, cycle * 4);

    // output positoin
    std::stringstream ss_pos;
    for (int i = 0; i < syndrome_lattice_x._num_node - 2; ++i) {
        Position pos = syndrome_lattice_x.index_to_position(i);
        ss_pos << pos.x << " " << pos.y << " " << pos.z << " " << (int)(error_lattice.is_anomaly_pos(pos, anomaly_size, anomaly_pos)) << std::endl;
    }
    for (int i = 0; i < syndrome_lattice_z._num_node - 2; ++i) {
        Position pos = syndrome_lattice_z.index_to_position(i);
        ss_pos << pos.x << " " << pos.y << " " << pos.z << " " << (int)(error_lattice.is_anomaly_pos(pos, anomaly_size, anomaly_pos)) << std::endl;
    }
    std::ofstream ofs_pos(filename + ".pos", std::ios::out);
    ofs_pos << ss_pos.str();
    ofs_pos.close();

    std::stringstream ss;
    for (unsigned int i = 0; i < trial_count; ++i) {
        // create error and parity
        auto error_info = error_lattice.generate_sample_anomaly_region_long(random, error_prob_anomaly_ratio, anomaly_size, anomaly_pos);
        ErrorSample error_sample = error_info.first;
        uint8_t correct_parity = error_info.second;

        // create syndrome
        auto detected_nodes_x = syndrome_lattice_x.create_symdrome_return_list(error_sample, error_lattice);
        auto detected_nodes_z = syndrome_lattice_z.create_symdrome_return_list(error_sample, error_lattice);

        for (int i = 0; i < detected_nodes_x.size(); ++i) {
            ss << (int)(detected_nodes_x[i]);
            if (i + 1 < detected_nodes_x.size()) ss << " ";
        }
        ss << " ";
        for (int i = 0; i < detected_nodes_z.size(); ++i) {
            ss << (int)(detected_nodes_z[i]);
            if (i + 1 < detected_nodes_z.size()) ss << " ";
        }
        ss << std::endl;
    }
    std::ofstream ofs(filename, std::ios::out);
    ofs << ss.str();
    ofs.close();
}

int main(int argc, char** argv) {
    std::string filename = "test.txt";
    unsigned int distance = 21;
    unsigned int cycle = 10;
    unsigned int trial_count = 100;
    double error_prob = 1e-3;
    double anomaly_size = 10;
    double error_prob_anomaly_ratio = 1e2;
    int anomaly_pos = 5;

    if (argc >= 2 && argc != 9) {
        printf("filename, distance, cycle, trial_count, error_prob, anomaly_ratio, anomaly_size, anomaly_pos\n");
        exit(0);
    }
    if (argc >= 2) {
        filename = std::string(argv[1]);
        distance = atoi(argv[2]);
        cycle = atoi(argv[3]);
        trial_count = atoi(argv[4]);
        error_prob = atof(argv[5]);
        error_prob_anomaly_ratio = atof(argv[6]);
        anomaly_size = atoi(argv[7]);
        anomaly_pos = atoi(argv[8]);
    }
    printf("distance=%d cycle=%d trial_count=%d\n", distance, cycle, trial_count);

    anomaly_detect_region_long(filename, distance, cycle, trial_count, error_prob, error_prob_anomaly_ratio, anomaly_size, anomaly_pos);
}

