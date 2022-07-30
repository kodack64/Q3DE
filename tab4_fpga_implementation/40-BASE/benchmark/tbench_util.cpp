// Copyright 2022 NTT CORPORATION

#include "tbench.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>

#include <string>
#include <random>

using namespace std;

void load_lattice(string path, LatticeInfo& lattice_info) {
    // Load lattice information from file
    stringstream ss;
    ss << path;
    if (lattice_info.stab == StabType::STAB_X) ss << "stabx";
    else ss << "stabz";
    ss << "_distance_" << lattice_info.d << "_cycle_" << lattice_info.cyc;
    string file_prefix = ss.str();

    string buf;
    int line_index = 0;

    ifstream ifs_node(file_prefix + ".node");
    if (!ifs_node) {
        cerr << "file not found" << endl;
        exit(1);
    }
    line_index = 0;
    while (getline(ifs_node, buf)) {
        stringstream st(buf);
        int32_t index, x, y, z;
        st >> index >> x >> y >> z;
        assert(index == line_index);
        line_index += 1;

        LatticeNode node;
        node.x = x; node.y = y; node.z = z;
        lattice_info.nodes.push_back(node);
    }
    ifs_node.close();

    ifstream ifs_bnd_path(file_prefix + ".boundary_path");
    if (!ifs_bnd_path) {
        cerr << "file not found" << endl;
        exit(1);
    }
    line_index = 0;
    while (getline(ifs_bnd_path, buf)) {
        stringstream st(buf);
        int32_t index, bnd_len;
        string bnd_name;
        st >> index >> bnd_name >> bnd_len;
        assert(index == line_index);
        line_index += 1;

        lattice_info.nodes[index].nearest_boundary_name = bnd_name;
        lattice_info.nodes[index].nearest_boundary_length = bnd_len;
    }
    ifs_bnd_path.close();
    assert(line_index == lattice_info.nodes.size());

    ifstream ifs_bnd(file_prefix + ".boundary");
    if (!ifs_bnd) {
        cerr << "file not found" << endl;
        exit(1);
    }
    while (getline(ifs_bnd, buf)) {
        stringstream st(buf);
        int32_t index;
        string bnd_name;
        st >> index >> bnd_name;
        lattice_info.boundary.insert(make_pair(index, bnd_name));
    }
    ifs_bnd.close();


    ifstream ifs_edge(file_prefix + ".edge");
    if (!ifs_edge) {
        cerr << "file not found" << endl;
        exit(1);
    }
    while (getline(ifs_edge, buf)) {
        stringstream st(buf);
        int32_t node1, node2, x, y, z;
        st >> node1 >> node2 >> x >> y >> z;
        LatticeEdge edge;
        edge.x = x;
        edge.y = y;
        edge.z = z;
        edge.t1 = node1;
        edge.t2 = node2;
        lattice_info.edges.push_back(edge);
    }
    ifs_edge.close();

    lattice_info.set_inverse();
}

void sample(const LatticeInfo& lattice_info, ErrorInfo& error_info, double error_prob, uint32_t seed) {
    // sampling error information from lattice information
    mt19937 mt(seed);
    uniform_real_distribution<double> dist(0.0, 1.0);
    error_info.seed = seed;
    error_info.init();
    for (size_t edge_index = 0;edge_index < lattice_info.edges.size(); edge_index++) {
        double value = dist(mt);
        if (value < error_prob) {
            error_info.errors[edge_index] = 1;
            const auto& edge = lattice_info.edges.at(edge_index);
            error_info.flip(edge.t1);
            error_info.flip(edge.t2);
        }
    }
}

std::vector<NodeInfo> extract_raw_nodes(const LatticeInfo& lattice_info, const ErrorInfo& error_info) {
    std::vector<NodeInfo> raw_nodes;
    for (int32_t synd_index = 0; synd_index < error_info.syndromes.size(); ++synd_index) {
        if (error_info.syndromes[synd_index]) {
            NodeInfo node;
            auto synd = lattice_info.nodes[synd_index];
            node.index = synd_index;
            node.x = synd.x / 2;
            node.y = synd.y / 2;
            node.z = synd.z / 2 + 1;
            node.d_b = synd.nearest_boundary_length;
            raw_nodes.push_back(node);
        }
    }
    std::sort(raw_nodes.begin(), raw_nodes.end(), [](NodeInfo& lhs, NodeInfo& rhs) { return std::tie(lhs.z, lhs.y, lhs.x) < tie(rhs.z, rhs.y, rhs.x); });
    return raw_nodes;
}

bool compare(vector<pair<int, int>> a, vector<pair<int, int>> b) {
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); ++i) {
        if (a[i].first != b[i].first) return false;
        if (a[i].second != b[i].second) return false;
    }
    return true;
}
