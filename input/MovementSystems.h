#pragma once

#include "../tiling/GridMaker.h"
#include "InputHandler.h"
#include "MovementEnums.h"

/*
 * Take in movement directions and adjust for collision and physics
 */

Vector2f topDownMovement(Vector2f start, Vector2f move, Vector2i size, Indexer *collision);
Vector2f topDownMovement(Node *node, Vector2f move, Indexer *collision);
Vector2f topDownMovement(Node *node, Vector2f move, Indexer *collision, double distance);

struct GlobalPhysicsStats {
	int jumpPower = 340;
	int jumpBoost = 8;
	int fallSpeed = 48;
	float fallMax = 600;
	int slideSpeed = 200;
	int slideMax = 200;
	int slideReverse = 2;
	int pushPower = 20;
};

class PersonalPhysicsStats {
public:
	bool showDebug = false;

	Vector2f previous = Vector2f(0,0);
	Vector2f pushDirection = Vector2f(0,0);
	Vector2f nodePosition = Vector2f(0,0);
	Vector2i nodeSize = Vector2i(1,1);

	float snapSpeed = 2;
	float jumpTime = 0;
	float weight = 0;
	float pushWeight = 0;
	bool blocked = false;
};

class PhysicsObject {
public:
	virtual PersonalPhysicsStats *getPhysics() = 0;
};

Vector2f platformFrictionMovement(Vector2f start, Vector2f move, Vector2i size, double time,
	Vector2f previous, Indexer *collision, Indexer *frictionMap, float frictionValue,
	GlobalPhysicsStats *globalPhysics);
Vector2f platformGravityMovement(Vector2f start, Vector2f move, Vector2i size, double time, bool jumpInput,
	Indexer *collision, GlobalPhysicsStats *globalPhysics, PersonalPhysicsStats *physics, std::vector<PersonalPhysicsStats *> colliding);