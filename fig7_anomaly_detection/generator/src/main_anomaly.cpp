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
#include "decoder.hpp"

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

void calculate(std::string filename, int distance, int cycle, uint32_t trial_count, double error_prob, double error_prob_anomaly_happen, double error_prob_anomaly_ratio) {
    Random random;
#ifdef _DEBUG
    unsigned int seed = 0;
    random.set_seed(seed);
#else
    unsigned int seed = random.set_random_seed();
#endif

    ErrorLattice error_lattice(distance, cycle);
    set_error_lattice_uniform(error_prob / 3, error_prob / 3, error_prob / 3, error_lattice);

    SyndromeLatticeX syndrome_lattice_x(distance, cycle);
    SyndromeLatticeZ syndrome_lattice_z(distance, cycle);
    syndrome_lattice_x.create_weighted_graph_uniform();
    syndrome_lattice_z.create_weighted_graph_uniform();

#ifdef DEBUG_FLAG
    unsigned int error_count_total = 0;
    unsigned int error_position_total = 0;
    error_lattice.debug_print_lattice();
    syndrome_lattice_x.debug_print_lattice();
    syndrome_lattice_z.debug_print_lattice();
#endif
    printf("configure finish\n");


    Decoder decoder;
    unsigned int logical_x_count = 0;
    unsigned int logical_y_count = 0;
    unsigned int logical_z_count = 0;
    for (unsigned int i = 0; i < trial_count; ++i) {
        // create error and parity
        auto error_info = error_lattice.generate_sample(random, error_prob_anomaly_happen, error_prob_anomaly_ratio);
        //auto error_info = error_lattice.generate_sample_constant_anomaly(random, error_prob_anomaly_happen, error_prob_anomaly_ratio);
        ErrorSample error_sample = error_info.first;
        uint8_t correct_parity = error_info.second;

        // create syndrome
        SyndromeSample detected_nodes_x = syndrome_lattice_x.create_symdrome(error_sample, error_lattice);
        SyndromeSample detected_nodes_z = syndrome_lattice_z.create_symdrome(error_sample, error_lattice);

        // decode
        MatchingResult matching_x = decoder.decode(detected_nodes_x, syndrome_lattice_x);
        MatchingResult matching_z = decoder.decode(detected_nodes_z, syndrome_lattice_z);

        // check parity
        uint8_t answer_parity_x = 0;
        for (auto match : matching_x) {
            uint8_t left_parity = syndrome_lattice_x.count_check_parity(match.first, match.second);
            answer_parity_x ^= left_parity;
        }
        uint8_t answer_parity_z = 0;
        for (auto match : matching_z) {
            uint8_t top_parity = syndrome_lattice_z.count_check_parity(match.first, match.second);
            answer_parity_z ^= top_parity;
        }
        uint8_t answer_parity = answer_parity_x + answer_parity_z * 2;

        bool success = false;
        if (correct_parity == answer_parity) {
            success = true;
        }
        else if (correct_parity == (answer_parity ^ 1)) {
            logical_x_count++;
        }
        else if (correct_parity == (answer_parity ^ 2)) {
            logical_z_count++;
        }
        else {
            logical_y_count++;
        }

#ifdef DEBUG_FLAG
        printf("***** %d/%d trial *****\n", i, trial_count);
        unsigned int error_count = 0;
        for (auto val : error_sample) {
            if (val / 2) error_count++;
            if (val % 2) error_count++;
        }
        error_count_total += error_count;
        error_position_total += (unsigned int)error_sample.size();

        unsigned int matching_cost_x = 0;
        for (auto match : matching_x) {
            matching_cost_x += syndrome_lattice_x.get_cost(match.first, match.second);
        }

        unsigned int matching_cost_z = 0;
        for (auto match : matching_z) {
            matching_cost_z += syndrome_lattice_z.get_cost(match.first, match.second);
        }


        if (!success || error_count < matching_cost_x + matching_cost_z) {
            syndrome_lattice_x.debug_print_sample(detected_nodes_x, error_sample, error_lattice);
            for (auto match : matching_x) {
                syndrome_lattice_x.debug_print_matching(match.first, match.second);
            }
            printf("////////////////////\n");
            for (auto n1 : detected_nodes_x) {
                for (auto n2 : detected_nodes_x) {
                    if (n1 >= syndrome_lattice_x._num_node - 2)continue;
                    if (n1 != n2)
                        syndrome_lattice_x.debug_print_matching(n1, n2);
                }
            }
            printf("parity correct%d : answer%d\n", correct_parity % 2, answer_parity_x);
            printf("error_count %d/%d (%lf) , matching_cost %d\n", error_count, (int)error_sample.size(), 1.*error_count / error_sample.size(), matching_cost_x);

            //////

            syndrome_lattice_z.debug_print_sample(detected_nodes_z, error_sample, error_lattice);
            for (auto match : matching_z) {
                syndrome_lattice_z.debug_print_matching(match.first, match.second);
            }
            printf("////////////////////\n");
            for (auto n1 : detected_nodes_z) {
                for (auto n2 : detected_nodes_z) {
                    if (n1 >= syndrome_lattice_z._num_node - 2)continue;
                    if (n1 != n2)
                        syndrome_lattice_z.debug_print_matching(n1, n2);
                }
            }
            printf("parity correct%d : answer%d\n", correct_parity / 2, answer_parity_z);
            printf("error_count %d/%d (%lf) , matching_cost %d\n", error_count, (int)error_sample.size(), 1.*error_count / error_sample.size(), matching_cost_z);

#ifdef _MSC_VER
            _getch();
#endif
        }

#endif
    }

    FILE* fp = fopen(filename.c_str(), "a");
    printf("distance=%d cycle=%d seed = %u\n", distance, cycle, seed);
    double logical_x_error_prob = 1.*logical_x_count / trial_count;
    double logical_y_error_prob = 1.*logical_y_count / trial_count;
    double logical_z_error_prob = 1.*logical_z_count / trial_count;
    printf("error=(%lf, %lf, %lf) -> logigcal_error=(%lf, %lf, %lf) (%u,%u,%u)/%u\n",
        error_prob, error_prob_anomaly_happen, error_prob_anomaly_ratio,
        logical_x_error_prob, logical_y_error_prob, logical_z_error_prob,
        logical_x_count, logical_y_count, logical_z_count, trial_count);
#ifdef DEBUG_FLAG
    printf("error count %d/%d (%lf)\n", error_count_total, error_position_total, 1.*error_count_total / error_position_total);
#endif
    fprintf(fp, "%d %d %u %lf %lf %lf %d %d %d %d\n",
        distance, cycle, seed,
        error_prob, error_prob_anomaly_ratio, error_prob_anomaly_happen,
        trial_count, logical_x_count, logical_y_count, logical_z_count);
    fclose(fp);
}



void anomaly_detect(std::string filename, int distance, int cycle, uint32_t trial_count, double error_prob, double error_prob_anomaly_happen, double error_prob_anomaly_ratio) {
    Random random;
#ifdef _DEBUG
    unsigned int seed = 0;
    random.set_seed(seed);
#else
    unsigned int seed = random.set_random_seed();
#endif

    ErrorLattice error_lattice(distance, cycle);
    set_error_lattice_uniform(error_prob / 3, error_prob / 3, error_prob / 3, error_lattice);

    SyndromeLatticeX syndrome_lattice_x(distance, cycle);
    SyndromeLatticeZ syndrome_lattice_z(distance, cycle);
    syndrome_lattice_x.create_weighted_graph_uniform();
    syndrome_lattice_z.create_weighted_graph_uniform();

#ifdef DEBUG_FLAG
    unsigned int error_count_total = 0;
    unsigned int error_position_total = 0;
    error_lattice.debug_print_lattice();
    syndrome_lattice_x.debug_print_lattice();
    syndrome_lattice_z.debug_print_lattice();
#endif
    printf("configure finish\n");


    Decoder decoder;
    std::ofstream ofs(filename, std::ios::app);
    for (unsigned int i = 0; i < trial_count; ++i) {
        // create error and parity
        auto error_info = error_lattice.generate_sample_constant_anomaly(random, error_prob_anomaly_happen, error_prob_anomaly_ratio);
        ErrorSample error_sample = error_info.first;
        uint8_t correct_parity = error_info.second;

        // create syndrome
        SyndromeSample detected_nodes_x = syndrome_lattice_x.create_symdrome(error_sample, error_lattice);
        SyndromeSample detected_nodes_z = syndrome_lattice_z.create_symdrome(error_sample, error_lattice);

        std::map<Position, int> counter_x, counter_z;
        for (auto value : detected_nodes_x) {
            if (value == syndrome_lattice_x._boundary_main_id || value == syndrome_lattice_x._boundary_sub_id) continue;
            Position pos = syndrome_lattice_x.index_to_position(value);
            pos.z = 0;
            counter_x[pos] += 1;
        }
        for (auto value : detected_nodes_z) {
            if (value == syndrome_lattice_z._boundary_main_id || value == syndrome_lattice_z._boundary_sub_id) continue;
            Position pos = syndrome_lattice_z.index_to_position(value);
            pos.z = 0;
            counter_z[pos] += 1;
        }


        // decode
        MatchingResult matching_x = decoder.decode(detected_nodes_x, syndrome_lattice_x);
        MatchingResult matching_z = decoder.decode(detected_nodes_z, syndrome_lattice_z);

        // check parity
        uint8_t answer_parity_x = 0;
        for (auto match : matching_x) {
            uint8_t left_parity = syndrome_lattice_x.count_check_parity(match.first, match.second);
            answer_parity_x ^= left_parity;
        }
        uint8_t answer_parity_z = 0;
        for (auto match : matching_z) {
            uint8_t top_parity = syndrome_lattice_z.count_check_parity(match.first, match.second);
            answer_parity_z ^= top_parity;
        }
        uint8_t answer_parity = answer_parity_x + answer_parity_z * 2;

        uint32_t result_parity = answer_parity ^ correct_parity;
        ofs << result_parity;
        //ofs << " X"; for (auto pair : counter_x) ofs << " " << pair.first.x << " " << pair.first.y << " " << pair.second;
        //ofs << " Z"; for (auto pair : counter_z) ofs << " " << pair.first.x << " " << pair.first.y << " " << pair.second;
        ofs << " X"; for (auto pair : counter_x) ofs << " " << pair.second;
        ofs << " Z"; for (auto pair : counter_z) ofs << " " << pair.second;
        ofs << std::endl;

    }
    ofs.close();
}


void anomaly_detect_region(std::string filename, int distance, int cycle, uint32_t trial_count, double error_prob, double error_prob_anomaly_ratio, int anomaly_size, int anomaly_pos) {
    Random random;
#ifdef _DEBUG
    unsigned int seed = 0;
    random.set_seed(seed);
#else
    unsigned int seed = random.set_random_seed();
#endif

    ErrorLattice error_lattice(distance, cycle);
    set_error_lattice_uniform(error_prob / 3, error_prob / 3, error_prob / 3, error_lattice);

    SyndromeLatticeX syndrome_lattice_x(distance, cycle);
    SyndromeLatticeZ syndrome_lattice_z(distance, cycle);
    //syndrome_lattice_x.create_weighted_graph_uniform();
    //syndrome_lattice_z.create_weighted_graph_uniform();

#ifdef DEBUG_FLAG
    unsigned int error_count_total = 0;
    unsigned int error_position_total = 0;
    //error_lattice.debug_print_lattice();
    //syndrome_lattice_x.debug_print_lattice();
    //syndrome_lattice_z.debug_print_lattice();
#endif
    printf("configure finish\n");


    Decoder decoder;
	std::stringstream ss;
    std::map<Position, int> counter_x, counter_z;

    for (unsigned int i = 0; i < trial_count; ++i) {
        // create error and parity
        auto error_info = error_lattice.generate_sample_anomaly_region(random, error_prob_anomaly_ratio, anomaly_size, anomaly_pos);
        ErrorSample error_sample = error_info.first;
        uint8_t correct_parity = error_info.second;

        // create syndrome
        SyndromeSample detected_nodes_x = syndrome_lattice_x.create_symdrome(error_sample, error_lattice);
        SyndromeSample detected_nodes_z = syndrome_lattice_z.create_symdrome(error_sample, error_lattice);

        if (counter_x.size() == 0) {
            for (int i = 0; i < syndrome_lattice_x._num_node-2; ++i) {
                Position pos = syndrome_lattice_x.index_to_position(i);
                pos.z = 0;
                counter_x[pos] = 0;
            }
            for (int i = 0; i < syndrome_lattice_z._num_node-2; ++i) {
                Position pos = syndrome_lattice_z.index_to_position(i);
                pos.z = 0;
                counter_z[pos] = 0;
            }
        }
        else {
            {
                auto ite = counter_x.begin();
                while (ite != counter_x.end()) {
                    ite->second = 0;
                    ite++;
                }
            }
            {
                auto ite = counter_z.begin();
                while (ite != counter_z.end()) {
                    ite->second = 0;
                    ite++;
                }
            }
        }

        for (auto value : detected_nodes_x) {
            if (value == syndrome_lattice_x._boundary_main_id || value == syndrome_lattice_x._boundary_sub_id) continue;
            Position pos = syndrome_lattice_x.index_to_position(value);
            pos.z = 0;
            counter_x[pos] += 1;
        }
        for (auto value : detected_nodes_z) {
            if (value == syndrome_lattice_z._boundary_main_id || value == syndrome_lattice_z._boundary_sub_id) continue;
            Position pos = syndrome_lattice_z.index_to_position(value);
            pos.z = 0;
            counter_z[pos] += 1;
        }

        {
            auto ite = counter_x.begin();
            while (ite != counter_x.end()) {
                ss << "X " << ite->first.x << " " << ite->first.y << " " << ite->second << std::endl;
                ite++;
            }
        }
        {
            auto ite = counter_z.begin();
            while (ite != counter_z.end()) {
                ss << "Z " << ite->first.x << " " << ite->first.y << " " << ite->second << std::endl;
                ite++;
            }
        }
    }
    std::ofstream ofs(filename, std::ios::app);
	ofs << ss.str();
    ofs.close();
}

int main(int argc, char** argv) {
    std::string filename = "test.txt";
    unsigned int distance = 21;
    unsigned int cycle = 10;
    unsigned int trial_count = 1000;
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

    //calculate(filename, distance, cycle, trial_count, error_prob, error_prob_anomaly_happen, error_prob_anomaly_ratio);
    anomaly_detect_region(filename, distance, cycle, trial_count, error_prob, error_prob_anomaly_ratio, anomaly_size, anomaly_pos);
}

