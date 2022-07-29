// Copyright 2022 NTT CORPORATION

#pragma once

#include <random>

class Random {
private:
	std::random_device rd;
	std::mt19937 mt;
	std::uniform_real_distribution<double> urd;
public:
	Random() {
		unsigned int seed = rd();
		mt.seed(seed);
	}
	virtual ~Random() {};
	void set_seed(unsigned int seed) {
		mt.seed(seed);
	}
	unsigned int set_random_seed() {
		unsigned int seed = rd();
		mt.seed(seed);
		return seed;
	}
	double sample_double() {
		return urd(mt);
	}
	int sample_integer() {
		return mt();
	}
};
