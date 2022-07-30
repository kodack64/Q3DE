// Copyright 2022 NTT CORPORATION

#pragma once

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <cassert>

struct LatticeNode{
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;
    std::string nearest_boundary_name = "";
    int32_t nearest_boundary_length = 0;
};

struct LatticeEdge {
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;
    int32_t t1 = 0;
    int32_t t2 = 0;
};

#ifdef _MSC_VER
enum class StabType {
#else
enum StabType {
#endif
    STAB_X, STAB_Z
};

struct LatticeInfo {
    const int32_t d;
    const int32_t cyc;
    const StabType stab;

    // list of nodes (syndromes)
    std::vector<LatticeNode> nodes;
    // list of edges with connecting two indices
    std::vector<LatticeEdge> edges;

    // map from boundary-node-index to boundary names
    std::map<int32_t, std::string> boundary;

    // inverse dictionary of nodes
    std::map<std::tuple<int32_t, int32_t, int32_t>, int32_t> node_inv;
    // inverse map of boundary
    std::map<std::string, int32_t> boundary_inv;

    LatticeInfo(int32_t distance, int32_t cycle, StabType stab_type) : d(distance), cyc(cycle), stab(stab_type) {};

    void set_inverse(){
        node_inv.clear();
        for(size_t i=0;i<nodes.size();++i){
            auto& node = nodes[i];
            auto tuple = std::make_tuple(node.x, node.y, node.z);
            node_inv.insert(std::make_pair(tuple, (int32_t)i));
        }

        boundary_inv.clear();
        for (auto ite = boundary.begin(); ite != boundary.end(); ++ite) {
            auto item = make_pair((*ite).second, (*ite).first);
            boundary_inv.insert(item);
        }
    }
};

struct ErrorInfo {
    // seed number to generate error information
    uint32_t seed = 0;
    // error is occurred or not
    std::vector<int8_t> errors;
    // observed syndrome values
    std::vector<int8_t> syndromes;
    // the parity of boundaries
    std::vector<int8_t> boundary;

    ErrorInfo(const LatticeInfo& lattice_info){
        syndromes = std::vector<int8_t>(lattice_info.nodes.size(), 0);
        errors = std::vector<int8_t>(lattice_info.edges.size(), 0);
        boundary = std::vector<int8_t>(lattice_info.boundary.size(), 0);
    }
    void init(){
        std::fill(errors.begin(), errors.end(), 0);
        std::fill(syndromes.begin(), syndromes.end(), 0);
        std::fill(boundary.begin(), boundary.end(), 0);
    }
    void flip(size_t node_index){
        if(node_index < syndromes.size()){
            syndromes.at(node_index) ^= 1;
        }else{
            size_t boundary_index = node_index - syndromes.size();
            boundary.at(boundary_index) ^= 1;
        }
    }
};

struct NodeInfo {
    int32_t index;
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t d_b;
};

struct AnomalyInfo {
    int lx;
    int rx;
    int ly;
    int ry;
    int lz;
    int rz;
    int bnd_len;
    AnomalyInfo(int _lx, int _rx, int _ly, int _ry, int _lz, int _rz, int d)
        :lx(_lx), rx(_rx), ly(_ly), ry(_ry), lz(_lz), rz(_rz) {
        assert(lx <= rx);
        assert(ly <= ry);
        assert(lz <= rz);
        bnd_len = std::min(lx, d - rx);
    }
    int get_med(int p, int l, int r) const {
        assert(l <= r);
        if (p < l) return l;
        if (p > r) return r;
        return p;
    }
    int point_to_anomaly(int px, int py, int pz) const {
        int ax = get_med(px, lx, rx);
        int ay = get_med(py, ly, ry);
        int az = get_med(pz, lz, rz);
        int len = abs(px - ax) + abs(py - ay) + abs(pz - az);
        return len;
    }
    std::tuple<int, int, int> nearest_anomaly(int px, int py, int pz) const {
        int ax = get_med(px, lx, rx);
        int ay = get_med(py, ly, ry);
        int az = get_med(pz, lz, rz);
        return std::make_tuple(ax, ay, az);
    }
};

void load_lattice(std::string name, LatticeInfo& lattice_info);
void sample(const LatticeInfo& lattice_info, ErrorInfo& error_info, double error_prob, uint32_t seed);
std::vector<NodeInfo> extract_raw_nodes(const LatticeInfo& lattice_info, const ErrorInfo& error_info);
bool compare(std::vector<std::pair<int, int>> pair_trial, std::vector<std::pair<int, int>> pair_correct);

std::vector<std::pair<int, int>> match_trial(const std::vector<NodeInfo>& nodes);
std::vector<std::pair<int, int>> match_greedy(const std::vector<NodeInfo>& nodes);
std::vector<std::pair<int, int>> match_iterative_greedy(const std::vector<NodeInfo>& nodes);

std::vector<std::pair<int, int>> match_trial_with_anomaly(const std::vector<NodeInfo>& nodes, const AnomalyInfo& anomaly_info);
std::vector<std::pair<int, int>> match_greedy_with_anomaly(const std::vector<NodeInfo>& nodes, const AnomalyInfo& anomaly_info);
std::vector<std::pair<int, int>> match_iterative_greedy_with_anomaly(const std::vector<NodeInfo>& nodes, const AnomalyInfo& anomaly_info);

void visualize_lattice(const LatticeInfo& lattice_info);
void visualize_error(const LatticeInfo& lattice_info, const ErrorInfo& error_info);
void visualize_match(const LatticeInfo& lattice_info, const ErrorInfo& error_info, const std::vector<NodeInfo>& node_info, const std::vector<std::pair<int, int>>& pair_trial, const std::vector<std::pair<int, int>>& pair_correct);
