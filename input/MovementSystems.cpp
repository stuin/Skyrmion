#include "MovementSystems.h"

sf::Vector2f topDownMovement(sf::Vector2f start, sf::Vector2f move, sf::Vector2i size, Indexer *collision) {
	sf::Vector2f end = start + move;

	if(collision != NULL && collision->getTile(end) != EMPTY) {
		sf::Vector2f horizontal = sf::Vector2f(start.x, end.y);
		sf::Vector2f vertical = sf::Vector2f(end.x, start.y);

		if(collision->getTile(horizontal) == EMPTY &&
			collision->getTile(vertical) == EMPTY)
			end = (abs(move.x) > abs(move.y)) ? horizontal : vertical;
		else if(collision->getTile(horizontal) == EMPTY)
			end = horizontal;
		else if(collision->getTile(vertical) == EMPTY)
			end = vertical;
		else
			end = start;
	}

	return end;
}

sf::Vector2f topDownMovement(Node *node, sf::Vector2f move, Indexer *collision) {
	return topDownMovement(node->getGPosition(), move, node->getSize(), collision);
}

sf::Vector2f topDownMovement(Node *node, sf::Vector2f move, Indexer *collision, double distance) {
	return topDownMovement(node->getGPosition(), vectorLength(move, distance), node->getSize(), collision);
}

sf::Vector2f platformFrictionMovement(sf::Vector2f start, sf::Vector2f move, sf::Vector2i size, double time,
	sf::Vector2f previous, Indexer *collision, Indexer *frictionMap, float frictionValue,
	GlobalPhysicsStats *globalPhysics) {

	sf::Vector2f foot = sf::Vector2f(start.x, start.y + size.y / 2 - 2);
	sf::Vector2f footL = foot - sf::Vector2f(size.x / 2, 0);
	sf::Vector2f footR = foot + sf::Vector2f(size.x / 2, 0);
	foot.y += 6;

	float netFriction = (frictionMap->getTile(foot) / 100.0) * frictionValue;
	netFriction = std::clamp(0.01f, netFriction, 1.0f);
	if(netFriction < 1 && (collision->getTile(foot) != EMPTY ||
		collision->getTile(footL) == SLOPE_UPLEFT || collision->getTile(footR) == SLOPE_UPRIGHT)) {

		move.x *= netFriction;
		move.x += previous.x * (1.0f - netFriction/3) * time;

		if((collision->getTile(footL) == SLOPE_UPLEFT || collision->getTile(foot) == SLOPE_UPLEFT) && move.x > -globalPhysics->slideReverse) {
			move.x += globalPhysics->slideSpeed * (1 - netFriction) * time;
			move.x = std::min(move.x, (float)(globalPhysics->slideMax * time));
		}
		if((collision->getTile(footR) == SLOPE_UPRIGHT || collision->getTile(foot) == SLOPE_UPRIGHT) && move.x < globalPhysics->slideReverse) {
			move.x -= globalPhysics->slideSpeed * (1 - netFriction) * time;
			move.x = std::max(move.x, -(float)(globalPhysics->slideMax * time));
		}
	}

	return move;
}

bool isAbove(sf::Vector2f position, sf::Vector2i size, sf::Vector2f otherPosition, sf::Vector2i otherSize) {
	float dx = position.x - otherPosition.x;
	float dy = (otherPosition.y - otherSize.y/2) - (position.y + size.y/4);
	float side = size.x/3 + otherSize.x/2;
	return std::abs(dx) < side && dy > 0;
}

sf::Vector2f platformGravityMovement(sf::Vector2f start, sf::Vector2f move, sf::Vector2i size, double time, bool jumpInput,
	Indexer *collision, GlobalPhysicsStats *globalPhysics, PersonalPhysicsStats *physics, std::vector<PersonalPhysicsStats *> colliding) {

	sf::Vector2f velocity = sf::Vector2f(move.x, 0);
	sf::Vector2f foot = sf::Vector2f(start.x, start.y + physics->nodeSize.y / 2 + 4);
	physics->nodePosition = start;
	physics->nodeSize = size;

	if(physics->showDebug)
		std::cout << velocity.x << "," << velocity.y << ">1 ";

	//Check for wall
	sf::Vector2f collisionOffset = velocity + sf::Vector2f(velocity.x / std::abs(velocity.x) * physics->nodeSize.x / 2, physics->nodeSize.y / 4);
	if(physics->previous.y != 0 || foot.y - 8 < collision->snapPosition(foot).y) {
		if(collision->getTile(start + collisionOffset) == FULL)
			velocity.x = 0;
		else if(velocity.x > 0 && collision->getTile(start) != SLOPE_UPLEFT && collision->getTile(start + collisionOffset) == SLOPE_UPLEFT)
			velocity.x = 0;
		else if(velocity.x < 0 && collision->getTile(start) != SLOPE_UPRIGHT && collision->getTile(start + collisionOffset) == SLOPE_UPRIGHT)
			velocity.x = 0;
		physics->blocked = std::abs(move.x) > 0.1 && std::abs(velocity.x) < 0.1;
	}

	if(physics->showDebug)
		std::cout << velocity.x << "," << velocity.y << ">2 ";

	//Falling and jumping
	foot += velocity;
	sf::Vector2f footL = foot - sf::Vector2f(physics->nodeSize.x / 4, 0);
	sf::Vector2f footR = foot + sf::Vector2f(physics->nodeSize.x / 4, 0);
	int tileL = collision->getTile(footL);
	int tileR = collision->getTile(footR);
	if(tileL == EMPTY && tileR == EMPTY) {
		if(physics->jumpTime > 0.2 || !jumpInput)
			physics->previous.y += globalPhysics->fallSpeed;
		physics->previous.y = std::min(physics->previous.y, globalPhysics->fallMax);
		velocity.y += physics->previous.y * time;

		if(physics->showDebug)
			std::cout << velocity.x << "," << velocity.y << ">3 ";

		//Ceiling check
		if(collision->getTile(start + velocity) != EMPTY && collision->getTile(start + velocity) != ONEWAY_UP) {
			physics->previous.y = 0;
			velocity.y = 0;
			physics->jumpTime += 0.2;
		}

		foot.y += physics->previous.y * time;
		physics->jumpTime += time;
	} else if(jumpInput && physics->jumpTime == 0) {
		//Start jump
		physics->previous.y = -globalPhysics->jumpPower;
		velocity.y += physics->previous.y * time;
	}

	if(physics->showDebug)
		std::cout << velocity.x << "," << velocity.y << ">4 ";

	//Snap to ground
	int tile = collision->getTile(foot);
	if(tile != EMPTY && !(jumpInput && physics->jumpTime < 0.2)) {

		sf::Vector2f ground = collision->snapPosition(foot);
		sf::Vector2f foot2 = foot - sf::Vector2f(0, 8);

		//Allow for upwards slope
		if(tile != EMPTY && tile != SLOPE_UPLEFT && tile != SLOPE_UPRIGHT &&
			(collision->getTile(foot2) == SLOPE_UPLEFT || collision->getTile(foot2) == SLOPE_UPRIGHT)) {
			foot = foot2;
			ground = collision->snapPosition(foot);
		}
		velocity.y += ground.y - start.y - physics->nodeSize.y/2;

		if(collision->getTile(foot) == SLOPE_UPLEFT)
			velocity.y -= ground.x - start.x;
		else if(collision->getTile(foot) == SLOPE_UPRIGHT)
			velocity.y -= start.x - ground.x - collision->getScale().x;

		velocity.y = std::min(std::max(physics->snapSpeed, physics->previous.y * (float)time), velocity.y);

		if(velocity.y == 0) {
			physics->previous.y = 0;
			physics->jumpTime = 0;
		}
	}

	if(physics->showDebug)
		std::cout << velocity.x << "," << velocity.y << ">5 ";

	std::vector<PersonalPhysicsStats *> pushing;
	physics->pushWeight = 0;
	for(PersonalPhysicsStats *other : colliding) {
		if(isAbove(start, size, other->nodePosition, other->nodeSize)) {
			if(jumpInput && physics->jumpTime == time) {
				physics->jumpTime = 0;
				physics->previous.y = -globalPhysics->jumpPower;
				velocity.y += physics->previous.y * time;
			} else if(velocity.y > 0 && other->previous.y >= 0) {
				velocity.y = (other->nodePosition.y - other->nodeSize.y/2) - (start.y + physics->nodeSize.y/2)+2;
				//velocity.y += other->previous.y * time;
				physics->jumpTime = 0;

				other->pushWeight += physics->pushWeight;
			}
		} else if(velocity.x != 0 && (other->nodePosition.x - start.x) * velocity.x > 0) {
			if(other->blocked || other->pushWeight > 1)
				velocity.x = 0;

			pushing.push_back(other);
			physics->pushWeight += other->pushWeight;
		}
	}
	colliding.clear();

	physics->pushWeight += physics->weight;
	velocity.x *= 1-std::clamp(0.0f, physics->pushWeight, 1.0f);

	physics->pushDirection = sf::Vector2f(0,0);
	for(PersonalPhysicsStats *other : pushing) {
		other->pushDirection.x = velocity.x / time;
		if(velocity.x > 0)
			velocity.x = std::min(other->previous.x * (float)time, velocity.x);
		else
			velocity.x = std::max(other->previous.x * (float)time, velocity.x);
	}

	if(physics->showDebug)
		std::cout << velocity.x << "," << velocity.y << ">6\n";

	physics->previous.x = velocity.x / time;
	return velocity;
}