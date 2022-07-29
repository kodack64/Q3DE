// Copyright 2022 NTT CORPORATION

#pragma once

#include <vector>
#include "syndrome_lattice_base.hpp"
#include "blossom5/PerfectMatching.h"

using MatchingResult = std::vector<std::pair<NodeIndex, NodeIndex>>;

class Decoder {
private:
public:
	Decoder() {};
	virtual ~Decoder() {};
	virtual MatchingResult decode(const SyndromeSample& detected_nodes, const SyndromeLattice& syndrome_lattice) {
		// create graph
		NodeIndex node_cnt = (NodeIndex)detected_nodes.size();
		EdgeIndex edge_cnt = node_cnt * (node_cnt - 1);
		PerfectMatching* pm = new PerfectMatching((signed)node_cnt, (signed)edge_cnt);
		for (NodeIndex i = 0; i < node_cnt; ++i) {
			for (NodeIndex j = i + 1; j < node_cnt; ++j) {
				WeightType weight = syndrome_lattice.get_weight(detected_nodes[i], detected_nodes[j]);
#ifdef PERFECT_MATCHING_DOUBLE
				pm->AddEdge(i, j, (PerfectMatching::REAL)weight);
#else
				// NOTE: if MWPM is int precision, add delta for avoiding rounded to small value
				pm->AddEdge(i, j, (PerfectMatching::REAL)(weight+1e-10));
#endif
			}
		}
		pm->options.verbose = false;
		pm->Solve();

		MatchingResult matching;
		for (NodeIndex i = 0; i < node_cnt; i++) {
			NodeIndex j = pm->GetMatch(i);
			if (j <= i) continue;
			matching.push_back(std::make_pair(detected_nodes[i], detected_nodes[j]));
		}
		delete pm;
		return matching;
	}
};

