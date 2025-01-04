#pragma once

#include "../tiling/GridMaker.h"
#include "InputHandler.h"

sf::Vector2f topDownMovement(sf::Vector2f start, sf::Vector2f move, sf::Vector2i size, Indexer *collision);
sf::Vector2f topDownMovement(Node *node, sf::Vector2f move, Indexer *collision);
sf::Vector2f topDownMovement(Node *node, sf::Vector2f move, Indexer *collision, double distance);

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

	sf::Vector2f previous = sf::Vector2f(0,0);
	sf::Vector2f pushDirection = sf::Vector2f(0,0);
	sf::Vector2f nodePosition = sf::Vector2f(0,0);
	sf::Vector2i nodeSize = sf::Vector2i(1,1);

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

sf::Vector2f platformFrictionMovement(sf::Vector2f start, sf::Vector2f move, sf::Vector2i size, double time,
	sf::Vector2f previous, Indexer *collision, Indexer *frictionMap, float frictionValue,
	GlobalPhysicsStats *globalPhysics);
sf::Vector2f platformGravityMovement(sf::Vector2f start, sf::Vector2f move, sf::Vector2i size, double time, bool jumpInput,
	Indexer *collision, GlobalPhysicsStats *globalPhysics, PersonalPhysicsStats *physics, std::vector<PersonalPhysicsStats *> colliding);