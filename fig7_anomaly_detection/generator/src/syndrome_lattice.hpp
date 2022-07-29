// Copyright 2022 NTT CORPORATION

#pragma once

#include "syndrome_lattice_base.hpp"

class SyndromeLatticeX : public SyndromeLattice{
public:
	SyndromeLatticeX(uint32_t distance, uint32_t cycle)
		: SyndromeLattice(distance, cycle, 1, 0, distance-1)
	{}
	virtual ~SyndromeLatticeX() {};

	virtual bool is_main_boundary_position(const Position& pos) const {
		unsigned int lattice_size = 2 * _distance - 1;
		unsigned int lattice_height = 2 * _cycle - 1;
		if (!(0 <= pos.y && pos.y < (signed)lattice_size)) return false;
		if (!(0 <= pos.z && pos.z < (signed)lattice_height)) return false;
		if (pos.z % 2 != 0) return false;
		if (pos.y % 2 != 0) return false;
		return pos.x == -1;
	}
	virtual NodeIndex count_edge_num_to_main_boundary(const Position& pos) const {
		return (pos.x + 1) / 2;
	}
	virtual bool is_sub_boundary_position(const Position& pos) const {
		unsigned int lattice_size = 2 * _distance - 1;
		unsigned int lattice_height = 2 * _cycle - 1;
		if (!(0 <= pos.y && pos.y < (signed)lattice_size)) return false;
		if (!(0 <= pos.z && pos.z < (signed)lattice_height)) return false;
		if (pos.z % 2 != 0) return false;
		if (pos.y % 2 != 0) return false;
		return pos.x == 2 * _distance - 1;
	}
	virtual NodeIndex count_edge_num_to_sub_boundary(const Position& pos) const {
		return _distance - (pos.x + 1) / 2;
	}
	virtual bool extract_error_bit(const ErrorSample& error_sample, EdgeIndex edge_index) const {
		return error_sample[edge_index] % 2;
	}
	virtual double extract_error_weight(const ErrorLattice& error_lattice, EdgeIndex edge_index) const {
		return error_lattice.get_error_prob_X(edge_index) + error_lattice.get_error_prob_Y(edge_index);
	}
};


class SyndromeLatticeZ : public SyndromeLattice {
public:
	SyndromeLatticeZ(uint32_t distance, uint32_t cycle)
		: SyndromeLattice(distance, cycle, 0, 1, distance)
	{}
	virtual ~SyndromeLatticeZ() {};

	virtual bool is_main_boundary_position(const Position& pos) const {
		unsigned int lattice_size = 2 * _distance - 1;
		unsigned int lattice_height = 2 * _cycle - 1;
		if (!(0 <= pos.x && pos.x < (signed)lattice_size)) return false;
		if (!(0 <= pos.z && pos.z < (signed)lattice_height)) return false;
		if (pos.z % 2 != 0) return false;
		if (pos.x % 2 != 0) return false;
		return pos.y == -1;
	}
	virtual NodeIndex count_edge_num_to_main_boundary(const Position& pos) const {
		return (pos.y + 1) / 2;
	}
	virtual bool is_sub_boundary_position(const Position& pos) const {
		unsigned int lattice_size = 2 * _distance - 1;
		unsigned int lattice_height = 2 * _cycle - 1;
		if (!(0 <= pos.x && pos.x < (signed)lattice_size)) return false;
		if (!(0 <= pos.z && pos.z < (signed)lattice_height)) return false;
		if (pos.z % 2 != 0) return false;
		if (pos.x % 2 != 0) return false;
		return pos.y == 2 * _distance - 1;
	}
	virtual NodeIndex count_edge_num_to_sub_boundary(const Position& pos) const {
		return _distance - (pos.y + 1) / 2;
	}
	virtual bool extract_error_bit(const ErrorSample& error_sample, EdgeIndex edge_index) const {
		return error_sample[edge_index] / 2;
	}
	virtual double extract_error_weight(const ErrorLattice& error_lattice, EdgeIndex edge_index) const {
		return error_lattice.get_error_prob_Z(edge_index) + error_lattice.get_error_prob_Y(edge_index);
	}
};
