#pragma once

enum TileCollisionTypes {
	EMPTY,
	FULL,
	SLOPE_UPLEFT,
	SLOPE_UPRIGHT,
	SLOPE_DOWNLEFT,
	SLOPE_DOWNRIGHT,
	ONEWAY_UP,
	ONEWAY_LEFT,
	ONEWAY_DOWN,
	ONEWAY_RIGHT
};

enum MovementDirections {
	UP,
	LEFT,
	DOWN,
	RIGHT
};

std::map<int, int> operator+(const std::map<int, int> &first, const std::map<int, int> &second);