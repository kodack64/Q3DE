// Copyright 2022 NTT CORPORATION

#include "tbench.h"
#include <iostream>

using namespace std;

int32_t get_cost(NodeInfo n1, NodeInfo n2) {
    return std::min(n1.d_b + n2.d_b, abs(n1.x - n2.x) + abs(n1.y - n2.y) + abs(n1.z - n2.z));
}

vector<pair<int, int>> match_greedy(const vector<NodeInfo>& nodes) {
    // create expected pair
    vector<int> expected_pair(nodes.size(), -2);
    vector<pair<int, int>> pairs;
    for (int index = 0; index < nodes.size(); ++index) {
        // skip if already matched
        if (expected_pair[index] != -2) continue;

        // get minimum cost target
        int min_cost = 2 * nodes[index].d_b;
        int min_target = index;
        for (int target = index + 1; target < nodes.size(); ++target) {
            if (expected_pair[target] != -2) continue;

            int cost = get_cost(nodes[index], nodes[target]);
            if (cost < min_cost) {
                min_cost = cost;
                min_target = target;
            }
        }
        if (min_target == index) {
            // matched to boundary
            expected_pair[index] = -1;
            pairs.push_back(make_pair(index, -1));
        }
        else {
            // matched to another node
            expected_pair[index] = min_target;
            expected_pair[min_target] = index;
            pairs.push_back(make_pair(index, min_target));
        }
    }
    sort(pairs.begin(), pairs.end());
    return pairs;
}

vector<pair<int, int>> match_iterative_greedy_org(const vector<NodeInfo>& nodes) {
    vector<int> expected_pair(nodes.size(), -2);
    vector<pair<int, int>> pairs;

    // increase allowed cost from 1 to d
    for (int allowed_cost = 1; allowed_cost <= 20; ++allowed_cost) {
        for (int index = 0; index < nodes.size(); ++index) {
            // skip if already matched
            if (expected_pair[index] != -2) continue;

            // get minimum cost target
            int min_cost = 2 * nodes[index].d_b;
            int min_target = index;
            for (int target = index + 1; target < nodes.size(); ++target) {
                if (expected_pair[target] != -2) continue;
                int cost = get_cost(nodes[index], nodes[target]);
                if (cost < min_cost) {
                    min_cost = cost;
                    min_target = target;
                }
            }

            // skip if min_cost is too large
            if (min_cost > allowed_cost)
                continue;

            if (min_target == index) {
                // matched to boundary
                expected_pair[index] = -1;
                pairs.push_back(make_pair(index, -1));
            }
            else {
                // matched to another node
                expected_pair[index] = min_target;
                expected_pair[min_target] = index;
                pairs.push_back(make_pair(index, min_target));
            }
        }
    }
    sort(pairs.begin(), pairs.end());
    return pairs;
}


vector<pair<int, int>> match_iterative_greedy(const vector<NodeInfo>& nodes) {
    vector<int> expected_pair(nodes.size(), -2);
    vector<pair<int, int>> pairs;

    vector<pair<NodeInfo,int>> node_queue;
    vector<int> is_queued(nodes.size(), 0);
    vector<int> is_used(nodes.size(), 0);
    int current_z = 0;
    int num_processed = 0;

    // loop until all the nodes are processed
    while(num_processed < nodes.size()){

    	// enqueue current layer
        for (int index = 0; index < nodes.size(); ++index){
        	if(is_queued[index]) continue;
        	if(nodes[index].z >= current_z) continue;

        	is_queued[index] = 1;
        }

        // match from older nodes
        //for (int index = 0; index < nodes.size(); ++index){
        for (int index = nodes.size()-1; index >= 0; --index){
        	if(is_used[index]) continue;
        	if(!is_queued[index]) continue;
        	int min_cost = 2 * nodes[index].d_b;
        	int min_target = index;

        	// find low-cost target from newer ones
        	for(int target=0;target < index; ++target){
        	//for(int target=nodes.size()-1;target >= index+1; --target){
            	if(is_used[target]) continue;
            	if(!is_queued[target]) continue;
        		int cost = get_cost(nodes[index], nodes[target]);
        		//if (cost < min_cost){
        		if (cost < min_cost || (cost == min_cost && min_target==index)){
        			min_cost = cost;
        			min_target = target;
        		}
        	}

        	// skip if cost is not allowed
        	int allowed_cost1 = current_z - nodes[index].z;
        	if(min_cost > allowed_cost1) continue;
        	int allowed_cost2 = current_z - nodes[min_target].z;
        	if(min_cost > allowed_cost2) continue;

        	// matched to boundary
        	if(min_target == index){
        		pairs.push_back(make_pair(index, -1));
        		is_used[index] = 1;
        		num_processed += 1;
        	}
        	// matched to another nodes
        	else{
        		pairs.push_back(make_pair(min(index, min_target), max(index, min_target)));
        		is_used[index] = 1;
        		is_used[min_target] = 1;
        		num_processed += 2;
        	}
        }
        current_z += 1;
    }
    sort(pairs.begin(), pairs.end());
    /*
    for(auto pair : pairs){
    	cout << pair.first << " - " << pair.second << endl;
    }
    */
    return pairs;
}


int32_t get_cost_with_anomaly(NodeInfo n1, const AnomalyInfo& anomaly_info) {
    int boundary_direct = n1.d_b;
    int boundary_anomaly = anomaly_info.bnd_len + anomaly_info.point_to_anomaly(n1.x, n1.y, n1.z);
    return min(boundary_direct, boundary_anomaly);
}

int32_t get_cost_with_anomaly(NodeInfo n1, NodeInfo n2, const AnomalyInfo& anomaly_info) {
    int boundary_path = get_cost_with_anomaly(n1, anomaly_info) + get_cost_with_anomaly(n2, anomaly_info);
    int manhattan_direct = abs(n1.x - n2.x) + abs(n1.y - n2.y) + abs(n1.z - n2.z);
    int manhattan_anomaly = anomaly_info.point_to_anomaly(n1.x, n1.y, n1.z) + anomaly_info.point_to_anomaly(n2.x, n2.y, n2.z);
    int manhattan = min(manhattan_direct, manhattan_anomaly);
    int length = min(boundary_path, manhattan);

    /*
    printf("ano:(%d,%d,%d)-(%d,%d,%d) len=%d ", anomaly_info.lx, anomaly_info.ly, anomaly_info.lz, anomaly_info.rx, anomaly_info.ry, anomaly_info.rz, anomaly_info.bnd_len);
    auto p1 = anomaly_info.nearest_anomaly(n1.x, n1.y, n1.z);
    auto p2 = anomaly_info.nearest_anomaly(n2.x, n2.y, n2.z);
    printf("n1:(%d,%d,%d)-(%d,%d,%d) len=%d ",n1.x, n1.y, n1.z, get<0>(p1), get<1>(p1), get<2>(p1), anomaly_info.point_to_anomaly(n1.x, n1.y, n1.z));
    printf("n2:(%d,%d,%d)-(%d,%d,%d) len=%d ",n2.x, n2.y, n2.z, get<0>(p2), get<1>(p2), get<2>(p2), anomaly_info.point_to_anomaly(n2.x, n2.y, n2.z));
    printf("manh_ano=%d bnd_ano = (%d,%d)\n", manhattan_anomaly, get_cost_with_anomaly(n1, anomaly_info), get_cost_with_anomaly(n2, anomaly_info));
    */
    return length;
}


vector<pair<int, int>> match_greedy_with_anomaly(const vector<NodeInfo>& nodes, const AnomalyInfo& anomaly_info) {
    // create expected pair
    vector<int> expected_pair(nodes.size(), -2);
    vector<pair<int, int>> pairs;
    for (int index = 0; index < nodes.size(); ++index) {
        // skip if already matched
        if (expected_pair[index] != -2) continue;

        // get minimum cost target
        int min_cost = 2 * get_cost_with_anomaly(nodes[index], anomaly_info);
        int min_target = index;
        for (int target = index + 1; target < nodes.size(); ++target) {
            if (expected_pair[target] != -2) continue;

            int cost = get_cost_with_anomaly(nodes[index], nodes[target], anomaly_info);
            if (cost < min_cost) {
                min_cost = cost;
                min_target = target;
            }
        }
        if (min_target == index) {
            // matched to boundary
            expected_pair[index] = -1;
            pairs.push_back(make_pair(index, -1));
        }
        else {
            // matched to another node
            expected_pair[index] = min_target;
            expected_pair[min_target] = index;
            pairs.push_back(make_pair(index, min_target));
        }
    }
    sort(pairs.begin(), pairs.end());
    return pairs;
}

vector<pair<int, int>> match_iterative_greedy_with_anomaly(const vector<NodeInfo>& nodes, const AnomalyInfo& anomaly_info) {
    vector<int> expected_pair(nodes.size(), -2);
    vector<pair<int, int>> pairs;

    // increase allowed cost from 1 to d
    for (int allowed_cost = 1; allowed_cost <= 20; ++allowed_cost) {
        for (int index = 0; index < nodes.size(); ++index) {
            // skip if already matched
            if (expected_pair[index] != -2) continue;

            // get minimum cost target
            int min_cost = 2 * get_cost_with_anomaly(nodes[index], anomaly_info);
            int min_target = index;
            for (int target = index + 1; target < nodes.size(); ++target) {
                if (expected_pair[target] != -2) continue;
                int cost = get_cost_with_anomaly(nodes[index], nodes[target], anomaly_info);
                if (cost < min_cost) {
                    min_cost = cost;
                    min_target = target;
                }
            }

            // skip if min_cost is too large
            if (min_cost > allowed_cost)
                continue;

            if (min_target == index) {
                // matched to boundary
                expected_pair[index] = -1;
                pairs.push_back(make_pair(index, -1));
            }
            else {
                // matched to another node
                expected_pair[index] = min_target;
                expected_pair[min_target] = index;
                pairs.push_back(make_pair(index, min_target));
            }
        }
    }
    sort(pairs.begin(), pairs.end());
    return pairs;
}
