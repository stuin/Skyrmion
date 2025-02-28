#pragma once

#include <map>

/*
 * General purpose directional enums
 */

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
	UP_RIGHT,
	RIGHT,
	DOWN_RIGHT,
	DOWN,
	DOWN_LEFT,
	LEFT,
	UP_LEFT
};

