// Copyright 2022 NTT CORPORATION

#pragma once

#include <vector>
#include "error_lattice.hpp"

using WeightType = double;
using WeightedGraph = std::vector<std::vector<WeightType>>;
using NodeIndex = uint32_t;
using SyndromeSample = std::vector<NodeIndex>;

class SyndromeLattice {
protected:
	const WeightType weight_inf = 1<<29;
	WeightedGraph _weighted_graph;
	WeightedGraph _weighted_graph_no_boundary;
	WeightedGraph _weighted_graph_two_boundary;

	void warshall_floyd(WeightedGraph& graph) const {
		assert(graph.size() == _num_node);
		for (unsigned int i = 0; i < _num_node; i++) {
			for (unsigned int j = 0; j < _num_node; j++) {
				for (unsigned int k = 0; k < _num_node; k++) {
					double dec = graph[j][i] + graph[i][k];;
					if (graph[j][k] > dec) {
						graph[j][k] = dec;
						graph[k][j] = dec;
					}
				}
			}
		}
	}

	virtual void set_weight(WeightedGraph& graph, NodeIndex n1, NodeIndex n2, WeightType weight) const {
		graph[n1][n2] = graph[n2][n1] = weight;
	}

public:
    const NodeIndex _boundary_main_id;
    const NodeIndex _boundary_sub_id;

    const NodeIndex _num_node;
	const NodeIndex _num_node_layer;
	const NodeIndex _num_node_width;
	const uint32_t _distance;
	const uint32_t _cycle;
	const uint32_t _offset_x;
	const uint32_t _offset_y;

	SyndromeLattice(uint32_t distance, uint32_t cycle, uint32_t offset_x, uint32_t offset_y, uint32_t num_node_width)
		: _distance(distance), _cycle(cycle),
		_num_node((distance - 1)*distance*cycle + 2),
		_num_node_layer((distance - 1)*distance),
		_boundary_main_id((distance - 1)*distance*cycle),
		_boundary_sub_id((distance - 1)*distance*cycle + 1),
		_offset_x(offset_x), _offset_y(offset_y), _num_node_width(num_node_width)
	{
		assert(distance > 0);
		assert(cycle > 0);
	}
	virtual ~SyndromeLattice() {}

	virtual bool is_exist(const Position& pos) const {
		int x, y, z;
		x = pos.x; y = pos.y; z = pos.z;
		unsigned int lattice_size = 2 * _distance - 1;
		unsigned int lattice_height = 2 * _cycle - 1;
		if (!(0 <= x && x < (signed)lattice_size)) return false;
		if (!(0 <= y && y < (signed)lattice_size)) return false;
		if (!(0 <= z && z < (signed)lattice_height)) return false;
		if (pos.x % 2 != _offset_x) return false;
		if (pos.y % 2 != _offset_y) return false;
		if (pos.z % 2 == 1) return false;
		return true;
	}
	virtual Position index_to_position(NodeIndex index) const {
		assert(0 <= index && index < _num_node);
		assert(index != _boundary_main_id);
		assert(index != _boundary_sub_id);
		int x, y, z;
		z = 2 * (index / _num_node_layer);
		index = index % _num_node_layer;
		y = 2 * (index / _num_node_width) + _offset_y;
		index = index % _num_node_width;
		x = 2 * index + _offset_x;
		return Position(x, y, z);
	}

	virtual NodeIndex position_to_index(const Position& pos) const {
		assert(is_exist(pos));
		NodeIndex result = 0;
		result += _num_node_layer*(pos.z / 2);
		result += _num_node_width*(pos.y / 2);
		result += pos.x / 2;
		return result;
	}
	virtual NodeIndex count_edge_num_between_nodes(const Position& pos1, const Position& pos2) const {
		return abs(pos1.x - pos2.x) / 2 + abs(pos1.y - pos2.y) / 2 + abs(pos1.z - pos2.z) / 2;
	}

	virtual bool is_main_boundary_position(const Position& pos) const = 0;
	virtual NodeIndex count_edge_num_to_main_boundary(const Position& pos) const = 0;
	virtual bool is_sub_boundary_position(const Position& pos) const = 0;
	virtual NodeIndex count_edge_num_to_sub_boundary(const Position& pos) const = 0;
	virtual bool extract_error_bit(const ErrorSample& error_sample, EdgeIndex edge_index) const = 0;
	virtual double extract_error_weight(const ErrorLattice& error_sample, EdgeIndex edge_index) const = 0;


	virtual SyndromeSample create_symdrome(const ErrorSample& error_sample, const ErrorLattice& error_lattice) const {
		// create syndrome
		const int8_t dx[] = { -1,1,0,0,0,0 };
		const int8_t dy[] = { 0,0,-1,1,0,0 };
		const int8_t dz[] = { 0,0,0,0,-1,1 };
		uint8_t direction = 6;

		SyndromeSample detected_nodes;
		for (NodeIndex index = 0; index < _num_node; ++index) {
			if (index == _boundary_main_id || index == _boundary_sub_id) continue;

			Position pos = index_to_position(index);
			bool synd = false;
			for (uint8_t dir = 0; dir < direction; ++dir) {
				Position edge_pos(pos);
				edge_pos.x += dx[dir];
				edge_pos.y += dy[dir];
				edge_pos.z += dz[dir];
				if (!error_lattice.is_exist(edge_pos, false)) continue;
				//printf("(%d,%d,%d)->(%d,%d,%d)\n", edge_pos.x, edge_pos.y, edge_pos.z, org_pos.x,org_pos.y,org_pos.z );
				EdgeIndex edge_index = error_lattice.position_to_index(edge_pos, true);
				synd ^= extract_error_bit(error_sample, edge_index);
			}
			if (synd) {
				detected_nodes.push_back(index);
			}
		}
		if (detected_nodes.size() % 2 == 1) {
			detected_nodes.push_back(_boundary_main_id);
		}
		return detected_nodes;
	}

    virtual std::vector<uint8_t> create_symdrome_return_list(const ErrorSample& error_sample, const ErrorLattice& error_lattice) const {
        // create syndrome
        const int8_t dx[] = { -1,1,0,0,0,0 };
        const int8_t dy[] = { 0,0,-1,1,0,0 };
        const int8_t dz[] = { 0,0,0,0,-1,1 };
        uint8_t direction = 6;

        std::vector<uint8_t> node_list;
        for (NodeIndex index = 0; index < _num_node; ++index) {
            if (index == _boundary_main_id || index == _boundary_sub_id) continue;

            Position pos = index_to_position(index);
            bool synd = false;
            for (uint8_t dir = 0; dir < direction; ++dir) {
                Position edge_pos(pos);
                edge_pos.x += dx[dir];
                edge_pos.y += dy[dir];
                edge_pos.z += dz[dir];
                if (!error_lattice.is_exist(edge_pos, false)) continue;
                //printf("(%d,%d,%d)->(%d,%d,%d)\n", edge_pos.x, edge_pos.y, edge_pos.z, org_pos.x,org_pos.y,org_pos.z );
                EdgeIndex edge_index = error_lattice.position_to_index(edge_pos, true);
                synd ^= extract_error_bit(error_sample, edge_index);
            }
            node_list.push_back(synd);
        }
        return node_list;
    }

	virtual void create_weighted_graph(const ErrorLattice& error_lattice, bool uniform_weight = false) {
		const int8_t dx[] = { -2,2,0,0,0,0 };
		const int8_t dy[] = { 0,0,-2,2,0,0 };
		const int8_t dz[] = { 0,0,0,0,-2,2 };
		const uint8_t direction = 6;

		_weighted_graph.clear();
		_weighted_graph.resize(_num_node, std::vector<WeightType>(_num_node, weight_inf));
		for (NodeIndex index = 0; index < _num_node - 2; ++index) {
			auto pos = index_to_position(index);
			for (uint8_t dir = 0; dir < direction; ++dir) {
				Position targ_pos(pos);
				targ_pos.x += dx[dir];
				targ_pos.y += dy[dir];
				targ_pos.z += dz[dir];
				NodeIndex targ_index = 0;
				if (is_main_boundary_position(targ_pos)) {
					targ_index = _boundary_main_id;
				}
				else if (is_sub_boundary_position(targ_pos)) {
					targ_index = _boundary_sub_id;
				}
				else {
					if (!is_exist(targ_pos)) continue;
					targ_index = position_to_index(targ_pos);
				}

				Position edge_pos(pos);
				edge_pos.x += dx[dir] / 2;
				edge_pos.y += dy[dir] / 2;
				edge_pos.z += dz[dir] / 2;
				EdgeIndex edge_index = error_lattice.position_to_index(edge_pos, true);
				double prob = extract_error_weight(error_lattice, edge_index);
				//printf("edge (%d,%d,%d), %lf\n", edge_pos.x, edge_pos.y, edge_pos.z, prob);
				assert(prob != 0.);
				assert(prob <= 1.0);
				if (prob == 0.) continue;

				assert(targ_index != index);
				assert(targ_index > index || _weighted_graph[targ_index][index] != weight_inf);
				if (targ_index > index) {
					if (uniform_weight) {
						set_weight(_weighted_graph, index, targ_index, 1);
					}
					else {
						set_weight(_weighted_graph, index, targ_index, -log(prob));
					}
				}
			}
		}

		// in normal weighted graph, left and right boundary is the same node
		set_weight(_weighted_graph, _boundary_main_id, _boundary_sub_id, 0);

		// in two_boundary graph, there are two boundary nodes, but they are not connected
		_weighted_graph_two_boundary = _weighted_graph;
		set_weight(_weighted_graph_two_boundary, _boundary_main_id, _boundary_sub_id, weight_inf);

		// in no_boundary graph, two boundary nodes are isolated
		_weighted_graph_no_boundary = _weighted_graph_two_boundary;
		for (NodeIndex index = 0; index < _num_node; ++index) {
			if (index != _boundary_main_id)
				set_weight(_weighted_graph_no_boundary, _boundary_main_id, index, weight_inf);
			if (index != _boundary_sub_id)
				set_weight(_weighted_graph_no_boundary, _boundary_sub_id, index, weight_inf);
		}

		warshall_floyd(_weighted_graph);
		warshall_floyd(_weighted_graph_two_boundary);
		warshall_floyd(_weighted_graph_no_boundary);
	}

	virtual void create_weighted_graph_uniform() {
		_weighted_graph.clear();
		_weighted_graph.resize(_num_node, std::vector<WeightType>(_num_node, weight_inf));
		for (NodeIndex n1 = 0; n1 < _num_node - 2; ++n1) {
			Position pos1 = index_to_position(n1);
			WeightType distance_to_main = count_edge_num_to_main_boundary(pos1);
			WeightType distance_to_sub = count_edge_num_to_sub_boundary(pos1);
			set_weight(_weighted_graph, n1, _boundary_main_id, distance_to_main);
			set_weight(_weighted_graph, n1, _boundary_sub_id, distance_to_sub);
			set_weight(_weighted_graph, n1, n1, 0);
		}
		_weighted_graph_two_boundary = _weighted_graph;
		_weighted_graph_no_boundary = _weighted_graph;

		for (NodeIndex n1 = 0; n1 < _num_node - 2; ++n1) {
			for (NodeIndex n2 = n1 + 1; n2 < _num_node - 2; ++n2) {
				Position pos1 = index_to_position(n1);
				Position pos2 = index_to_position(n2);
				WeightType mahattan_cost = count_edge_num_between_nodes(pos1, pos2);
				WeightType boundary_cost_main1 = _weighted_graph[n1][_boundary_main_id];
				WeightType boundary_cost_sub1 = _weighted_graph[n1][_boundary_sub_id];
				WeightType boundary_cost_main2 = _weighted_graph[n2][_boundary_main_id];
				WeightType boundary_cost_sub2 = _weighted_graph[n2][_boundary_sub_id];
				WeightType boundary_cost = std::min(boundary_cost_main1, boundary_cost_sub1) + std::min(boundary_cost_main2, boundary_cost_sub2);
				WeightType boundary_cost_separate = std::min(boundary_cost_main1 + boundary_cost_main2, boundary_cost_sub1 + boundary_cost_sub2);
				set_weight(_weighted_graph, n1, n2, std::min(mahattan_cost, boundary_cost));
				set_weight(_weighted_graph_two_boundary, n1, n2, std::min(mahattan_cost, boundary_cost_separate));
				set_weight(_weighted_graph_no_boundary, n1, n2, mahattan_cost);
			}
		}
		for (NodeIndex n1 = 0; n1 < _num_node - 2; ++n1) {
			Position pos1 = index_to_position(n1);
			WeightType distance_to_main = count_edge_num_to_main_boundary(pos1);
			WeightType distance_to_sub = count_edge_num_to_sub_boundary(pos1);
			WeightType distance_to_boundary = std::min(distance_to_main, distance_to_sub);
			set_weight(_weighted_graph, n1, _boundary_main_id, distance_to_boundary);
			set_weight(_weighted_graph, n1, _boundary_sub_id, distance_to_boundary);
			set_weight(_weighted_graph_no_boundary, n1, _boundary_main_id, weight_inf);
			set_weight(_weighted_graph_no_boundary, n1, _boundary_sub_id, weight_inf);
		}
		// in normal weighted graph, left and right boundary is the same node
		set_weight(_weighted_graph, _boundary_main_id, _boundary_sub_id, 0);
		set_weight(_weighted_graph_two_boundary, _boundary_main_id, _boundary_sub_id, _distance);
		set_weight(_weighted_graph_no_boundary, _boundary_main_id, _boundary_sub_id, weight_inf);
	}
	virtual WeightType get_weight(NodeIndex n1, NodeIndex n2) const {
		assert(n1 < _num_node && n2 < _num_node);
		return _weighted_graph[n1][n2];
	}
	virtual WeightType get_weight_without_boundary(NodeIndex n1, NodeIndex n2) const {
		assert(n1 < _num_node && n2 < _num_node);
		return _weighted_graph_no_boundary[n1][n2];
	}
	virtual WeightType get_weight_separate_boundary(NodeIndex n1, NodeIndex n2) const {
		assert(n1 < _num_node && n2 < _num_node);
		return _weighted_graph_two_boundary[n1][n2];
	}


	virtual uint8_t count_check_parity(NodeIndex n1, NodeIndex n2) const {
		uint8_t check_parity = 0;
		if (n2 < n1) std::swap(n1, n2);
		assert(n2 > n1);
		assert(n1 != _boundary_main_id && n1 != _boundary_sub_id);
		WeightType weight = _weighted_graph[n1][n2];
		WeightType weight_nobnd = _weighted_graph_no_boundary[n1][n2];
		WeightType n1_main = _weighted_graph_two_boundary[n1][_boundary_main_id];
		WeightType n1_sub = _weighted_graph_two_boundary[n1][_boundary_sub_id];
		WeightType n2_main = _weighted_graph_two_boundary[n2][_boundary_main_id];
		WeightType n2_sub = _weighted_graph_two_boundary[n2][_boundary_sub_id];
		if (n2 == _boundary_main_id || n2 == _boundary_sub_id) {
			// matched to boundary
			if (n1_main < n1_sub) check_parity ^= 1;
		}
		else {
			// matched to nodes
			if (abs(weight - weight_nobnd) > 1e-14) {
				if (n1_main < n1_sub) check_parity ^= 1;
				if (n2_main < n2_sub) check_parity ^= 1;
			}
		}
		return check_parity;
	}

	virtual void debug_print_lattice() const {
		printf("////////////////\n");
		for (unsigned int z = 0; z < 2 * _cycle; ++z) {
			for (unsigned int y = 0; y < 2 * _distance; ++y) {
				for (unsigned int x = 0; x < 2 * _distance; ++x) {
					Position pos(x, y, z);
					if (is_exist(pos)) {
						NodeIndex index = position_to_index(pos);
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
	}

	virtual void debug_print_sample(const SyndromeSample& detected_nodes, const ErrorSample& error_sample, const ErrorLattice& error_lattice) const {
		for (unsigned int z = 0; z < 2 * _cycle; ++z) {
			for (unsigned int y = 0; y < 2 * _distance; ++y) {
				for (unsigned int x = 0; x < 2 * _distance; ++x) {
					Position pos(x, y, z);
					if (error_lattice.is_exist(pos)) {
						assert(!this->is_exist(pos));
						int index = error_lattice.position_to_index(pos, true);
						int val = extract_error_bit(error_sample, index);
						//int val = error_sample[index];
						if (val)
							printf("E%3d", index);
						else
							printf(".   ");
					}
					else if (this->is_exist(pos)) {
						int index = this->position_to_index(pos);
						if (find(detected_nodes.begin(), detected_nodes.end(), index) == detected_nodes.end()) {
							printf("*   ");
						}
						else {
							printf("D%3d", index);
						}
					}
					else {
						printf("----");
					}
				}
				printf("\n");
			}
			printf("/////\n");
		}
	}

	virtual unsigned int get_cost(NodeIndex n1, NodeIndex n2) const {
		WeightType weight = _weighted_graph[n1][n2];
		WeightType weight_nobnd = _weighted_graph_no_boundary[n1][n2];
		WeightType n1_main = _weighted_graph_two_boundary[n1][_boundary_main_id];
		WeightType n1_sub = _weighted_graph_two_boundary[n1][_boundary_sub_id];
		WeightType n2_main = _weighted_graph_two_boundary[n2][_boundary_main_id];
		WeightType n2_sub = _weighted_graph_two_boundary[n2][_boundary_sub_id];

		unsigned int cost = 0;
		Position pos1 = this->index_to_position(n1);
		if (n2 == _boundary_main_id || n2 == _boundary_sub_id) {
			// matched to boundary
			if (n1_main < n1_sub) cost += count_edge_num_to_main_boundary(pos1);
			else cost += count_edge_num_to_sub_boundary(pos1);
		}
		else {
			// matched to nodes
			Position pos2 = this->index_to_position(n2);
			if (abs(weight - weight_nobnd) > 1e-14) {
				if (n1_main < n1_sub) cost += count_edge_num_to_main_boundary(pos1);
				else cost += count_edge_num_to_sub_boundary(pos1);
				if (n2_main < n2_sub) cost += count_edge_num_to_main_boundary(pos2);
				else cost += count_edge_num_to_sub_boundary(pos2);
			}
			else {
				cost += count_edge_num_between_nodes(pos1, pos2);
			}
		}
		return cost;
	}

	virtual void debug_print_matching(NodeIndex n1, NodeIndex n2) const {
		WeightType weight = _weighted_graph[n1][n2];
		WeightType weight_nobnd = _weighted_graph_no_boundary[n1][n2];
		WeightType n1_main = _weighted_graph_two_boundary[n1][_boundary_main_id];
		WeightType n1_sub = _weighted_graph_two_boundary[n1][_boundary_sub_id];
		WeightType n2_main = _weighted_graph_two_boundary[n2][_boundary_main_id];
		WeightType n2_sub = _weighted_graph_two_boundary[n2][_boundary_sub_id];
		printf("pair (%3d,%3d) : ", n1, n2);
		printf("pathcomp (%lf, %lf) ", weight, weight_nobnd);
		printf("ms1 (%lf, %lf) ", n1_main, n1_sub);
		printf("ms2 (%lf, %lf) ", n2_main, n2_sub);
		printf("cost: %d ", this->get_cost(n1, n2));
		printf("parity: %d", this->count_check_parity(n1, n2));
		printf("\n");
	}
};
