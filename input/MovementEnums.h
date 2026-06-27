#pragma once

#include <map>

#include "../core/Event.h"

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
	DOWN,
	LEFT,
	RIGHT,
	UP_LEFT,
	DOWN_RIGHT,
	UP_RIGHT,
	DOWN_LEFT
};