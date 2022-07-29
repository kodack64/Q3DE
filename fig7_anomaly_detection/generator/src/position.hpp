// Copyright 2022 NTT CORPORATION

#pragma once

struct Position {
	Position(int _x, int _y, int _z) {
		x = _x; y = _y; z = _z;
	}
	int x;
	int y;
	int z;
    virtual bool operator < (const Position& rhs) const { return std::tie(this->x, this->y, this->z) < std::tie(rhs.x, rhs.y, rhs.z); }
};

