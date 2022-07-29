// Copyright 2022 NTT CORPORATION

#pragma once

#include <cassert>
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include "position.hpp"
#include "random.hpp"

using EdgeIndex = uint32_t;
using WeightType = double;
using ErrorSample = std::vector<uint8_t>;

class ErrorLattice {
private:
	EdgeIndex _num_edge_data_layer_horizontal;
	EdgeIndex _num_edge_data_layer_vertical;
	EdgeIndex _num_edge_data_layer;
	EdgeIndex _num_edge_meas_layer;
	EdgeIndex _num_edge_cycle;
	EdgeIndex _num_edge;
	std::vector<WeightType> _error_prob_X_list;
	std::vector<WeightType> _error_prob_Y_list;
	std::vector<WeightType> _error_prob_Z_list;
public:
	const uint32_t _distance;
	const uint32_t _cycle;
	ErrorLattice(uint32_t distance, uint32_t cycle)
		: _distance(distance), _cycle(cycle) {

		_num_edge_data_layer_horizontal = _distance * _distance;
		_num_edge_data_layer_vertical = (_distance-1) * (_distance - 1);
		_num_edge_data_layer = _num_edge_data_layer_horizontal + _num_edge_data_layer_vertical;
		_num_edge_meas_layer = (_distance - 1) * _distance * 2;

		_num_edge_cycle = _num_edge_data_layer + _num_edge_meas_layer;
		_num_edge = _num_edge_cycle * (cycle - 1) + _num_edge_data_layer;

		_error_prob_X_list = std::vector<WeightType>(_num_edge, 0.);
		_error_prob_Y_list = std::vector<WeightType>(_num_edge, 0.);
		_error_prob_Z_list = std::vector<WeightType>(_num_edge, 0.);
	}

	bool _is_exist_core(const Position& pos) const {
		int x = pos.x;
		int y = pos.y;
		int z = pos.z;
		unsigned int lattice_size = 2 * _distance - 1;
		unsigned int lattice_height = 2 * _cycle - 1;
		if (!(0 <= x && x < (signed)lattice_size)) return false;
		if (!(0 <= y && y < (signed)lattice_size)) return false;
		if (!(0 <= z && z < (signed)lattice_height)) return false;
		if (z % 2 == 0) {
			// _num_edge_data_layer
			if (x % 2 == 0) {
				// horizontal
				return (y % 2 == 0);
			}
			else {
				// vertical
				return (y % 2 == 1);
			}
		}
		else {
			// meas _num_edge_cycle
			//if (x % 2 == 0) return false;
			//else if (y % 2 == 1) return false;
			//else return true;
			return (x % 2 != y % 2);
		}
	}
	bool is_exist(const Position& pos, bool fail_assert = false) const {
		bool result = _is_exist_core(pos);
		if (fail_assert) {
			if (!result) {
				printf("Assert: Try to touch non-existing lattice (%d,%d,%d)\n", pos.x, pos.y, pos.z);
				assert(result);
			}
		}
		return result;
	}
	EdgeIndex position_to_index(const Position& pos, bool fail_assert) const {
		if (!this->is_exist(pos, fail_assert)) return -1;
		EdgeIndex result = 0;
		int x, y, z;
		x = pos.x; y = pos.y; z = pos.z;
		int layer_count = z / 2;
		result += layer_count * _num_edge_cycle;
		if (z % 2 == 0) {
			// data _num_edge_cycle
			if (x % 2 == 0) {
				// horizontal
				result += _distance * (y / 2);
				result += x / 2;
			}
			else {
				// vertical
				result += _num_edge_data_layer_horizontal;
				result += (_distance - 1)*(y / 2);
				result += x / 2;
			}
		}
		else {
			// meas _num_edge_cycle
			result += _num_edge_data_layer;
			if (pos.y % 2 == 0) {
				result += (_distance - 1)*(y / 2);
				result += x / 2;
			}
			else {
				result += _num_edge_meas_layer / 2;
				result += _distance*(y / 2);
				result += x / 2;
			}
		}
		assert(0 <= result && result < (signed)_num_edge);
		return result;
	}
	Position index_to_position(EdgeIndex index) const {
		assert(0 <= index && index < _num_edge);
		int x, y, z;
		x = y = z = 0;
		z = 2 * (index / _num_edge_cycle);
		index = index % _num_edge_cycle;
		if (index >= _num_edge_data_layer) {
			// meas _num_edge_cycle
			index -= _num_edge_data_layer;
			z += 1;
			if (index < _num_edge_meas_layer / 2) {
				y = (index / (_distance - 1)) * 2;
				x = (index % (_distance - 1)) * 2 + 1;
			}
			else {
				index -= _num_edge_meas_layer / 2;
				y = (index / _distance) * 2+1;
				x = (index % _distance) * 2;
			}
		}
		else {
			// data _num_edge_cycle
			if (index >= _num_edge_data_layer_horizontal) {
				// vertical
				index -= _num_edge_data_layer_horizontal;
				y = (index / (_distance - 1)) * 2 + 1;
				x = (index % (_distance - 1)) * 2 + 1;
			}
			else {
				// horizontal
				y = (index / _distance) * 2;
				x = (index % _distance) * 2;
			}
		}
		return Position(x, y, z);
	}
	void stack_error_prob_X(EdgeIndex edge_index, double value) {
		_error_prob_X_list[edge_index] += value;
	}
	void stack_error_prob_Y(EdgeIndex edge_index, double value) {
		_error_prob_Y_list[edge_index] += value;
	}
	void stack_error_prob_Z(EdgeIndex edge_index, double value) {
		_error_prob_Z_list[edge_index] += value;
	}
	WeightType get_error_prob_X(EdgeIndex edge_index) const {
		return _error_prob_X_list[edge_index];
	}
	WeightType get_error_prob_Y(EdgeIndex edge_index) const {
		return _error_prob_Y_list[edge_index];
	}
	WeightType get_error_prob_Z(EdgeIndex edge_index) const {
		return _error_prob_Z_list[edge_index];
	}

	std::pair<ErrorSample, uint8_t> generate_sample(Random& random, double anomaly_prob, double anomaly_ratio) const {
		ErrorSample vec(_num_edge, 0);
        uint32_t lattice_width = _distance * 2 - 1;
        uint32_t qubit_count = lattice_width * lattice_width;

        std::vector<uint8_t> anomaly(qubit_count, 0);

        uint8_t check_parity = 0;
		assert(_error_prob_X_list.size() == _num_edge);
		assert(_error_prob_Y_list.size() == _num_edge);
		assert(_error_prob_Z_list.size() == _num_edge);
        
        uint32_t update_cycle = _num_edge_data_layer + _num_edge_meas_layer;

		for (EdgeIndex i = 0; i < _num_edge; ++i) {

            if (i%update_cycle == 0) {
                for (uint32_t qubit_index = 0; qubit_index < qubit_count; ++qubit_index) {
                    double r = random.sample_double();
                    if (r < anomaly_prob) {
                        anomaly[qubit_index] = 1;
                    }
                }
            }

            Position pos = index_to_position(i);
            uint32_t qubit_index = pos.x + pos.y*lattice_width;
            uint8_t is_anomaly = anomaly[qubit_index];

            double r = random.sample_double();
            double px = get_error_prob_X(i);
            double py = get_error_prob_Y(i);
            double pz = get_error_prob_Z(i);
            if (is_anomaly) {
                px *= anomaly_ratio;
                py *= anomaly_ratio;
                pz *= anomaly_ratio;
            }

			if (r < px + py) {
				vec[i] ^= 1;
				if (pos.x == 0 && pos.z%2==0) {
					check_parity ^= 1;
				}
			}
			if (px <= r && r < px + py + pz) {
				vec[i] ^= 2;
				if (pos.y == 0 && pos.z%2==0) {
					check_parity ^= 2;
				}
			}
		}
		return make_pair(vec, check_parity);
	}


    std::pair<ErrorSample, uint8_t> generate_sample_constant_anomaly(Random& random, double anomaly_prob, double anomaly_ratio) const {
        ErrorSample vec(_num_edge, 0);
        uint32_t lattice_width = _distance * 2 - 1;
        uint32_t qubit_count = lattice_width * lattice_width;

        std::vector<uint8_t> anomaly(qubit_count, 0);

        std::vector<uint32_t> anomaly_check(qubit_count, 0);
        std::iota(anomaly_check.begin(), anomaly_check.end(), 0);
        std::random_device rd;
        std::mt19937 engine(rd());
        std::shuffle(anomaly_check.begin(), anomaly_check.end(), engine);
        for (uint32_t ind = 0; ind < (uint32_t)(qubit_count*anomaly_prob); ++ind) {
            uint32_t qubit_index = anomaly_check[ind];
            anomaly[qubit_index] = 1;
        }

        uint8_t check_parity = 0;
        assert(_error_prob_X_list.size() == _num_edge);
        assert(_error_prob_Y_list.size() == _num_edge);
        assert(_error_prob_Z_list.size() == _num_edge);

        for (EdgeIndex i = 0; i < _num_edge; ++i) {

            Position pos = index_to_position(i);
            uint32_t qubit_index = pos.x + pos.y*lattice_width;
            uint8_t is_anomaly = anomaly[qubit_index];

            double r = random.sample_double();
            double px = get_error_prob_X(i);
            double py = get_error_prob_Y(i);
            double pz = get_error_prob_Z(i);
            if (is_anomaly) {
                px *= anomaly_ratio;
                py *= anomaly_ratio;
                pz *= anomaly_ratio;
            }

            if (r < px + py) {
                vec[i] ^= 1;
                if (pos.x == 0 && pos.z % 2 == 0) {
                    check_parity ^= 1;
                }
            }
            if (px <= r && r < px + py + pz) {
                vec[i] ^= 2;
                if (pos.y == 0 && pos.z % 2 == 0) {
                    check_parity ^= 2;
                }
            }
        }
        return make_pair(vec, check_parity);
    }


    std::pair<ErrorSample, uint8_t> generate_sample_anomaly_region(Random& random, double anomaly_ratio, int anomaly_size, int anomaly_pos) const {
        ErrorSample vec(_num_edge, 0);
        uint32_t lattice_width = _distance * 2 - 1;
        uint32_t qubit_count = lattice_width * lattice_width;

        std::vector<uint8_t> anomaly_edge(_num_edge, 0);

        int32_t center = lattice_width / 2 + anomaly_pos;
        for (uint32_t ind = 0; ind < _num_edge; ++ind) {
            Position pos = index_to_position(ind);
            if(std::abs(pos.x-center) < anomaly_size && std::abs(pos.y-center) < anomaly_size)
                anomaly_edge[ind] = 1;
        }

        uint8_t check_parity = 0;
        assert(_error_prob_X_list.size() == _num_edge);
        assert(_error_prob_Y_list.size() == _num_edge);
        assert(_error_prob_Z_list.size() == _num_edge);

        for (EdgeIndex i = 0; i < _num_edge; ++i) {

            Position pos = index_to_position(i);
            uint8_t is_anomaly = anomaly_edge[i];

            double r = random.sample_double();
            double px = get_error_prob_X(i);
            double py = get_error_prob_Y(i);
            double pz = get_error_prob_Z(i);
            if (is_anomaly) {
                px *= anomaly_ratio;
                py *= anomaly_ratio;
                pz *= anomaly_ratio;
            }

            if (r < px + py) {
                vec[i] ^= 1;
                if (pos.x == 0 && pos.z % 2 == 0) {
                    check_parity ^= 1;
                }
            }
            if (px <= r && r < px + py + pz) {
                vec[i] ^= 2;
                if (pos.y == 0 && pos.z % 2 == 0) {
                    check_parity ^= 2;
                }
            }
        }
        return make_pair(vec, check_parity);
    }


    bool is_anomaly_pos(Position pos, int anomaly_size, int anomaly_pos) const {
        uint32_t lattice_width = _distance * 2 - 1;
        int32_t center = lattice_width / 2 + anomaly_pos;
        return (
            std::abs(pos.x - center) < anomaly_size 
            && std::abs(pos.y - center) < anomaly_size
            && _cycle / 2 <= pos.z 
            && pos.z < _cycle * 3 / 2
            );
    }

    std::pair<ErrorSample, uint8_t> generate_sample_anomaly_region_long(Random& random, double anomaly_ratio, int anomaly_size, int anomaly_pos) const {
        ErrorSample vec(_num_edge, 0);
        uint32_t lattice_width = _distance * 2 - 1;
        uint32_t qubit_count = lattice_width * lattice_width;

        std::vector<uint8_t> anomaly_edge(_num_edge, 0);

        for (uint32_t ind = 0; ind < _num_edge; ++ind) {
            Position pos = index_to_position(ind);
            if(is_anomaly_pos(pos, anomaly_size, anomaly_pos))
                anomaly_edge[ind] = 1;
        }

        uint8_t check_parity = 0;
        assert(_error_prob_X_list.size() == _num_edge);
        assert(_error_prob_Y_list.size() == _num_edge);
        assert(_error_prob_Z_list.size() == _num_edge);

        for (EdgeIndex i = 0; i < _num_edge; ++i) {

            Position pos = index_to_position(i);
            uint8_t is_anomaly = anomaly_edge[i];

            double r = random.sample_double();
            double px = get_error_prob_X(i);
            double py = get_error_prob_Y(i);
            double pz = get_error_prob_Z(i);
            if (is_anomaly) {
                //printf("%d %d %d / %d\n", pos.x, pos.y, pos.z, _cycle);
                px *= anomaly_ratio;
                py *= anomaly_ratio;
                pz *= anomaly_ratio;
            }

            if (r < px + py) {
                vec[i] ^= 1;
                if (pos.x == 0 && pos.z % 2 == 0) {
                    check_parity ^= 1;
                }
            }
            if (px <= r && r < px + py + pz) {
                vec[i] ^= 2;
                if (pos.y == 0 && pos.z % 2 == 0) {
                    check_parity ^= 2;
                }
            }
        }
        return make_pair(vec, check_parity);
    }
	void debug_print_lattice() const {
		printf("////////////////\n");
		for (unsigned int z = 0; z < 2 * _cycle; ++z) {
			for (unsigned int y = 0; y < 2 * _distance; ++y) {
				for (unsigned int x = 0; x < 2 * _distance; ++x) {
					Position pos(x, y, z);
					if (is_exist(pos, false)) {
						EdgeIndex index = position_to_index(pos, true);
						printf("%3d ", index);
						Position pos2 = index_to_position(index);
						assert(pos.x == pos2.x);
						assert(pos.y == pos2.y);
						assert(pos.z == pos2.z);
					}
					else {
						printf("****");
					}
				}
				printf("\n");
			}
			printf("////////////////\n");
		}
		printf("num_edge = %d\n", _num_edge);
	}
};
