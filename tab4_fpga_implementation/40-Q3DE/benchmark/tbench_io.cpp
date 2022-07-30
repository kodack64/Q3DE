// Copyright 2022 NTT CORPORATION

#include "tbench.h"

#include <iostream>
#include <cassert>

using namespace std;

#ifndef _MSC_VER
#include "../decoder.h"

#ifndef ANOMALY
// Dummy declaration of anomaly struct for non-anomaly build
typedef struct {
    point_t p; // position of the node
    uchar d_b; // distance to boundary
    point_t q; // nearest anomarous node
    uchar d_a; // distance to anomaly
    // uchar x_L; // left edge of anomaly
    // uchar x_R; // right edge of anomaly
} in_node_ano_t;
typedef hls::stream<in_node_ano_t> AXI_STREAM_ANO_IN;
#else
// Dummy declaration of normal struct for anomaly build
typedef struct {
	point_t p;
	uchar d_b;
} in_node_t;
typedef hls::stream<in_node_t> AXI_STREAM_IN;
#endif

#else
// These are declared for debug on MSVC
#define MAXCYCLE 1000
#define NUMENTRIES 70 // the case for d=11
typedef unsigned char uchar;
typedef unsigned short uint16_t;
typedef struct {
    uchar x;
    uchar y;
    uchar z;
} point_t;

typedef struct {
    point_t p;
    uchar d_b;
} in_node_t;

typedef struct {
    point_t p; // position of the node
    uchar d_b; // distance to boundary
    point_t q; // nearest anomarous node
    uchar d_a; // distance to anomaly
    // uchar x_L; // left edge of anomaly
    // uchar x_R; // right edge of anomaly
} in_node_ano_t;

typedef struct {
    point_t p1;
    point_t p2;
} ou_pair_t;
#endif

#define CYCLE_MOD (1UL<<8)

const vector<in_node_t> convert_to_in_node(const vector<NodeInfo>& raw_nodes) {
    vector<in_node_t> nodes;
    int z = 0;
    for (int i = 0; i < raw_nodes.size(); ++i) {
        const auto& cur = raw_nodes[i];
        int next_node_cycle = cur.z % CYCLE_MOD;
        if (z != next_node_cycle) {
            z = (z + 1) % CYCLE_MOD;
            while (z != cur.z % CYCLE_MOD) {
                in_node_t empty_node;
                empty_node.p.x = 255;
                empty_node.p.y = 0;
                empty_node.p.z = z;
                empty_node.d_b = 0;
                nodes.push_back(empty_node);
                z = (z + 1) % CYCLE_MOD;
            }
        }
        in_node_t current_node;
        current_node.p.x = cur.x;
        current_node.p.y = cur.y;
        current_node.p.z = next_node_cycle;
        current_node.d_b = cur.d_b;
        nodes.push_back(current_node);
    }
    while(nodes.size() < MAXCYCLE){
        in_node_t empty_node;
        empty_node.p.x = 255;
        empty_node.p.y = 0;
        empty_node.p.z = z;
        empty_node.d_b = 0;
        nodes.push_back(empty_node);
        z = (z + 1) % CYCLE_MOD;
    }
    assert(nodes.size() == MAXCYCLE);
    return nodes;
}

const vector<in_node_ano_t> convert_to_in_node_with_anomaly(const vector<NodeInfo>& raw_nodes, AnomalyInfo anomaly_info) {
    vector<in_node_ano_t> nodes;
    int z = 0;
    for (int i = 0; i < raw_nodes.size(); ++i) {
        const auto& cur = raw_nodes[i];
        int next_node_cycle = cur.z % CYCLE_MOD;
        if (z != next_node_cycle) {
            z = (z + 1) % CYCLE_MOD;
            while (z != cur.z % CYCLE_MOD) {
                in_node_ano_t empty_node;
                empty_node.p.x = 255;
                empty_node.p.y = 0;
                empty_node.p.z = z;
                empty_node.d_b = 0;
                empty_node.q.x = 0;
                empty_node.q.y = 0;
                empty_node.q.z = 0;
                empty_node.d_a = 0;
                nodes.push_back(empty_node);
                z = (z + 1) % CYCLE_MOD;
            }
        }
        in_node_ano_t current_node;
        current_node.p.x = cur.x;
        current_node.p.y = cur.y;
        current_node.p.z = next_node_cycle;
        current_node.d_b = cur.d_b;
        auto pos = anomaly_info.nearest_anomaly(cur.x, cur.y, cur.z);
        current_node.q.x = get<0>(pos);
        current_node.q.y = get<1>(pos);
        current_node.q.z = get<2>(pos) % CYCLE_MOD;
        current_node.d_a = anomaly_info.point_to_anomaly(cur.x, cur.y, cur.z);
        nodes.push_back(current_node);
    }
    while(nodes.size() < MAXCYCLE){
        in_node_ano_t empty_node;
        empty_node.p.x = 255;
        empty_node.p.y = 0;
        empty_node.p.z = z;
        empty_node.d_b = 0;
        empty_node.q.x = 0;
        empty_node.q.y = 0;
        empty_node.q.z = 0;
        empty_node.d_a = 0;
        nodes.push_back(empty_node);
        z = (z + 1) % CYCLE_MOD;
    }
    assert(nodes.size() == MAXCYCLE);
    return nodes;
}


int _get_node_index(const vector<NodeInfo>& nodes, vector<int>& consumed, point_t p){
	if(p.x==255 && p.y == 255 && p.z == 255) return -1;

	auto pos1 = make_tuple(p.x, p.y, p.z);

	for(int i=0;i<nodes.size();++i){
		if(consumed[i]==1) continue;
		auto pos2 = make_tuple(nodes[i].x, nodes[i].y,nodes[i].z);
		if(pos1==pos2){
			consumed[i] = 1;
			return i;
		}
	}
	assert(false);
	return -1;
}

const vector<pair<int, int>> convert_to_pairs(const vector<NodeInfo>& nodes, const vector<ou_pair_t>& out_pairs) {

	vector<ou_pair_t> sorted_pairs;
	for(int i=0;i<out_pairs.size();++i){
		ou_pair_t pair = out_pairs[i];
		if(tie(pair.p1.z, pair.p1.y, pair.p1.x) > tie(pair.p2.z, pair.p2.y, pair.p2.x)){
			swap(pair.p1, pair.p2);
		}
		sorted_pairs.push_back(pair);
	}
	sort(sorted_pairs.begin(), sorted_pairs.end(), [](ou_pair_t& n1, ou_pair_t& n2){
		return tie(n1.p1.z, n1.p1.y, n1.p1.x, n1.p2.z, n1.p2.y, n1.p2.x)
				< tie(n2.p1.z, n2.p1.y, n2.p1.x, n2.p2.z, n2.p2.y, n2.p2.x);
	});
	vector<pair<int, int>> pairs;
	vector<int> consumed(nodes.size(), 0);
	for(int i=0;i<sorted_pairs.size();++i){
		int ind1 = _get_node_index(nodes, consumed, sorted_pairs[i].p1);
		int ind2 = _get_node_index(nodes, consumed, sorted_pairs[i].p2);
		pairs.push_back(make_pair(ind1, ind2));
	}

	return pairs;
}

#ifndef _MSC_VER
void put_stream(AXI_STREAM_IN& istr, const vector<in_node_t>& nodes) {
    for (int i = 0; i < nodes.size(); ++i) {
        istr << nodes[i];
    }
}
void put_stream_with_anomaly(AXI_STREAM_ANO_IN& istr, const vector<in_node_ano_t>& nodes) {
    for (int i = 0; i < nodes.size(); ++i) {
        istr << nodes[i];
    }
}
const vector<ou_pair_t> get_stream(AXI_STREAM_OU& ostr) {
    vector<ou_pair_t> pairs;
    while (!ostr.empty()) {
        ou_pair_t test_ou;
        ostr >> test_ou;
        pairs.push_back(test_ou);
    }
    return pairs;
}
#endif


void debug_print(const vector<NodeInfo>& nodes, const vector<in_node_t>& in_nodes, const vector<pair<int, int>>& pairs){
	cout << " * node index *" << endl;
	for(int i=0;i<nodes.size();++i){
		cout << i << ": (" << nodes[i].x << "," << nodes[i].y << "," << nodes[i].z << ")" << endl;
	}
    cout << " * input stream *" << endl;
    for (int i = 0; i < in_nodes.size(); ++i) {
        if (in_nodes[i].p.x == 255) continue;
        cout << i << ": (" << (int)in_nodes[i].p.x << " " << (int)in_nodes[i].p.y << " " << (int)in_nodes[i].p.z << ") d_b=" << (int)in_nodes[i].d_b << endl;
    }
	cout << " * output stream *" << endl;
	for(int i=0;i<pairs.size();++i){
		cout << pairs[i].first;
		if(pairs[i].first != -1){
			cout << " (" << nodes[pairs[i].first].x << "," << nodes[pairs[i].first].y << "," << nodes[pairs[i].first].z << ")";
		} else {
			cout << " (bnd)";
		}
		cout << " - ";
		cout << pairs[i].second;
		if(pairs[i].second != -1){
			cout << " (" << nodes[pairs[i].second].x << "," << nodes[pairs[i].second].y << "," << nodes[pairs[i].second].z << ")";
		} else {
			cout << " (bnd)";
		}
		cout << endl;
	}
}



void debug_print_with_anomaly(const vector<NodeInfo>& nodes, const vector<in_node_ano_t>& in_nodes, const vector<pair<int, int>>& pairs, const AnomalyInfo& anomaly_info){
	cout << " * node index *" << endl;
	for(int i=0;i<nodes.size();++i){
		cout << i << ": (" << nodes[i].x << "," << nodes[i].y << "," << nodes[i].z << ")" << endl;
	}
    cout << " * input stream *" << endl;
    for (int i = 0; i < in_nodes.size(); ++i) {
        if (in_nodes[i].p.x == 255) continue;
        cout << i << ": (" << (int)in_nodes[i].p.x << " " << (int)in_nodes[i].p.y << " " << (int)in_nodes[i].p.z << ") d_b=" << (int)in_nodes[i].d_b
        		<< ": nearest anomaly pos (" << (int)in_nodes[i].q.x << " " << (int)in_nodes[i].q.y << " " << (int)in_nodes[i].q.z << ") d_a=" << (int)in_nodes[i].d_a	<< endl;
    }
    cout << " * anomaly info *" << endl;
	cout << "Rectangular from (" << (int)anomaly_info.lx << " " << (int)anomaly_info.ly << " " << (int)anomaly_info.lz << ") to ("
			<< (int)anomaly_info.rx << " " << (int)anomaly_info.ry << " " << (int)anomaly_info.rz << ") anomaly_to_boundary=" << (int)anomaly_info.bnd_len << endl;
	cout << " * output stream *" << endl;
	for(int i=0;i<pairs.size();++i){
		cout << pairs[i].first;
		if(pairs[i].first != -1){
			cout << " (" << nodes[pairs[i].first].x << "," << nodes[pairs[i].first].y << "," << nodes[pairs[i].first].z << ")";
		} else {
			cout << " (bnd)";
		}
		cout << " - ";
		cout << pairs[i].second;
		if(pairs[i].second != -1){
			cout << " (" << nodes[pairs[i].second].x << "," << nodes[pairs[i].second].y << "," << nodes[pairs[i].second].z << ")";
		} else {
			cout << " (bnd)";
		}
		cout << endl;
	}
}

vector<pair<int, int>> match_trial(const vector<NodeInfo>& nodes) {
#ifdef _MSC_VER
    auto pairs = match_greedy(nodes);
    return pairs;
#else
    AXI_STREAM_IN sin;
    AXI_STREAM_OU sout;
    auto in_nodes = convert_to_in_node(nodes);
    put_stream(sin, in_nodes);

#ifndef ANOMALY
    decoder(sin, sout);
#endif

    auto out_pairs = get_stream(sout);
    auto pairs = convert_to_pairs(nodes, out_pairs);
    debug_print(nodes, in_nodes, pairs);
    return pairs;
#endif
}


vector<pair<int, int>> match_trial_with_anomaly(const vector<NodeInfo>& nodes, const AnomalyInfo& anomaly_info) {
#ifdef _MSC_VER
    auto pairs = match_greedy_with_anomaly(nodes, anomaly_info);
    return pairs;
#else
    AXI_STREAM_ANO_IN sin;
    AXI_STREAM_OU sout;
    auto in_nodes = convert_to_in_node_with_anomaly(nodes, anomaly_info);
    put_stream_with_anomaly(sin, in_nodes);

#ifdef ANOMALY
    decoder(sin, sout);
#endif

    auto out_pairs = get_stream(sout);
    auto pairs = convert_to_pairs(nodes, out_pairs);
#ifdef DEBUG_PRINT
    debug_print_with_anomaly(nodes, in_nodes, pairs, anomaly_info);
#endif
    return pairs;
#endif
}


