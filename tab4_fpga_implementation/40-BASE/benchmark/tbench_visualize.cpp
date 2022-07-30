// Copyright 2022 NTT CORPORATION

#include "tbench.h"

#include <iostream>
using namespace std;

void visualize_lattice(const LatticeInfo& lattice_info) {
    int max_x = lattice_info.d * 2 + 1;
    int max_y = lattice_info.d * 2 + 1;
    int max_z = lattice_info.cyc * 2 - 1;

    map<tuple<int, int, int>, string> data;

    for (int synd_index = 0; synd_index < lattice_info.nodes.size(); ++synd_index) {
        auto& node = lattice_info.nodes[synd_index];
        auto pos = make_tuple(node.x, node.y, node.z);
        string name = node.nearest_boundary_name + to_string(node.nearest_boundary_length) + " ";
        data.insert(make_pair(pos, name));
    }
    for (int z = 0; z < max_z; ++z) {
        cout << "*** cycle " << z << " ***" << endl;
        for (int y = 0; y < max_y; ++y) {
            for (int x = 0; x < max_x; ++x) {
                auto pos = make_tuple(x, y, z);
                auto ite = lattice_info.node_inv.find(pos);
                if (ite != lattice_info.node_inv.end()) {
                    int index = ite->second;
                    int length = lattice_info.nodes[index].nearest_boundary_length;
                    string name = lattice_info.nodes[index].nearest_boundary_name;
                    cout << name << length;
                }
                else {
                    cout << ". ";
                }
            }
            cout << endl;
        }
    }
}

void visualize_error(const LatticeInfo& lattice_info, const ErrorInfo& error_info) {
    int max_x = lattice_info.d * 2 + 1;
    int max_y = lattice_info.d * 2 + 1;
    int max_z = lattice_info.cyc * 2 - 1;

    map<tuple<int, int, int>, string> data;

    for (int synd_index = 0; synd_index < error_info.syndromes.size(); ++synd_index) {
        if (error_info.syndromes.at(synd_index)) {
            auto& node = lattice_info.nodes[synd_index];
            auto pos = make_tuple(node.x, node.y, node.z);
            string name = node.nearest_boundary_name + to_string(node.nearest_boundary_length) + " ";
            data.insert(make_pair(pos, name));
        }
    }

    for (int error_index = 0; error_index < error_info.errors.size(); ++error_index) {
        if (error_info.errors.at(error_index)) {
            auto& edge = lattice_info.edges[error_index];
            auto pos = make_tuple(edge.x, edge.y, edge.z);
            string name = " E ";
            data.insert(make_pair(pos, name));
        }
    }

    cout << "*** sample seed = " << error_info.seed << " ***" << endl;
    for (int z = 0; z < max_z; ++z) {
        cout << "*** cycle " << z << " ***" << endl;
        for (int y = 0; y < max_y; ++y) {
            for (int x = 0; x < max_x; ++x) {
                auto pos = make_tuple(x, y, z);
                auto ite = data.find(pos);
                if (ite != data.end()) {
                    cout << ite->second;
                }
                else {
                    cout << " . ";
                }
            }
            cout << endl;
        }
    }
}

void assign_pair(const LatticeInfo& lattice_info, const vector<NodeInfo>& node_info, const vector<pair<int, int>>& pairs, map<tuple<int, int, int>, string>& data) {
    for (int pair_index = 0; pair_index < pairs.size(); ++pair_index) {
        if (pairs[pair_index].first != -1) {
            auto& node = lattice_info.nodes[node_info[pairs[pair_index].first].index];
            auto pos = make_tuple(node.x, node.y, node.z);
            data[pos] += (pair_index<10?"0":"") + to_string(pair_index);
        }
        if (pairs[pair_index].second != -1) {
            auto& node = lattice_info.nodes[node_info[pairs[pair_index].second].index];
            auto pos = make_tuple(node.x, node.y, node.z);
            data[pos] += (pair_index<10?"0":"") + to_string(pair_index);
        }
    }
}

void visualize_match(const LatticeInfo& lattice_info, const ErrorInfo& error_info, const vector<NodeInfo>& node_info, const vector<pair<int, int>>& pair_trial, const vector<pair<int, int>>& pair_correct) {
    int max_x = lattice_info.d * 2 + 1;
    int max_y = lattice_info.d * 2 + 1;
    int max_z = lattice_info.cyc * 2 - 1;

    map<tuple<int, int, int>, string> data;

    for (int synd_index = 0; synd_index < lattice_info.nodes.size(); ++synd_index) {
		auto& node = lattice_info.nodes[synd_index];
		auto pos = make_tuple(node.x, node.y, node.z);
		string name = " . ";
		data.insert(make_pair(pos, name));
    }

    for (int edge_index = 0; edge_index < lattice_info.edges.size(); ++edge_index) {
		auto& edge = lattice_info.edges[edge_index];
		auto pos = make_tuple(edge.x, edge.y, edge.z);
		string name = " . ";
		data.insert(make_pair(pos, name));
    }

    for (int synd_index = 0; synd_index < error_info.syndromes.size(); ++synd_index) {
        if (error_info.syndromes.at(synd_index)) {
            auto& node = lattice_info.nodes[synd_index];
            auto pos = make_tuple(node.x, node.y, node.z);
            string name = "S";
            data[pos] = name;
        }
    }

    for (int error_index = 0; error_index < error_info.errors.size(); ++error_index) {
        if (error_info.errors.at(error_index)) {
            auto& edge = lattice_info.edges[error_index];
            auto pos = make_tuple(edge.x, edge.y, edge.z);
            string name = " E ";
            data[pos] = name;
        }
    }

    map<tuple<int, int, int>, string> data_dup(data);
    assign_pair(lattice_info, node_info, pair_trial, data);
    assign_pair(lattice_info, node_info, pair_correct, data_dup);


    cout << "*** sample seed = " << error_info.seed << " ***" << endl;
    cout << "(Left) HLS codes (Right) C++ implementation" << endl;
    for (int z = 0; z < max_z; ++z) {
        cout << "*** cycle " << z << " ***" << endl;
        for (int y = 0; y < max_y; ++y) {
            for (int x = 0; x < max_x; ++x) {
                auto pos = make_tuple(x, y, z);
                auto ite = data.find(pos);
                if (ite != data.end()) {
                    cout << ite->second;
                }
                else {
                    cout << "   ";
                }
            }
            cout << "     ";
            for (int x = 0; x < max_x; ++x) {
                auto pos = make_tuple(x, y, z);
                auto ite = data_dup.find(pos);
                if (ite != data_dup.end()) {
                    cout << ite->second;
                }
                else {
                    cout << "   ";
                }
            }
            cout << endl;
        }
    }
}
