#include "Node.h"

/*
 * Created by Stuart Irwin on 4/16/2019.
 * Sprite with collision
 */

//Base constructor
Node::Node(Layer layer, sf::Vector2i size, bool hidden, Node *parent) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	this->layer = layer;

	setSize(size);
	setHidden(hidden);
	setParent(parent);
}

//Get node layer variable
int Node::getLayer() {
	return layer;
}

sf::Rect<int> Node::getRect() {
	sf::Vector2f v = this->getGPosition() - this->getOrigin();
	size.left = v.x;
	size.top = v.y;
	return size;
}

//Get parent node
Node *Node::getParent() {
	return parent;
}

//Get global position
sf::Vector2f Node::getGPosition() {
	if(parent != NULL)
		return getPosition() + parent->getGPosition();
	return getPosition();
}

//Create position in a direction and distance
sf::Vector2f Node::getShiftedPosition(sf::Vector2f dir, double distance) {
	float xOffset = 0;
	float yOffset = 0;
	if(dir.x == 0 && dir.y == 0)
		return getPosition();
	if(dir.x == 0)
		yOffset = copysign(distance, dir.y);
	else if(dir.y == 0)
		xOffset = copysign(distance, dir.x);
	else if(abs(dir.x) == abs(dir.y)) {
		float adjustment = sqrt(2) / 2.0;
		xOffset = copysign(distance, dir.x);
		yOffset = copysign(distance, dir.y);
	} else {
		float angle = std::atan2(dir.y, dir.x);
		xOffset = cos(angle) * distance;
		yOffset = sin(angle) * distance;
	}

	return sf::Vector2f(getPosition().x + xOffset, getPosition().y + yOffset);
}

//Check if node is hidden
bool Node::isHidden() {
	return hidden || deleted || (parent != NULL && parent->isHidden());
}

//Set collision box size
void Node::setSize(sf::Vector2i size) {
	this->size.width = size.x;
	this->size.height = size.y;
	setOrigin(size.x / 2, size.y / 2);
}

//Set whether node is hidden
void Node::setHidden(bool hidden) {
	this->hidden = hidden;
}

//Set parent node
void Node::setParent(Node *parent) {
	this->parent = parent;
}

void Node::setGPosition(float x, float y) {
	sf::Vector2f pos(x, y);
	if(parent != NULL)
		pos -= parent->getGPosition();
	setPosition(pos);
}

//Get full collision bitset
std::bitset<MAXLAYER> Node::getCollisionLayers() {
	return collisionLayers;
}

//Check if node collides with layer
bool Node::getCollisionLayer(Layer layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return collisionLayers[layer];
}

//Set if node collides with layer
void Node::collideWith(Layer layer, bool collide) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	collisionLayers[layer] = collide;
}

//Check collision box against other node
bool Node::checkCollision(Node *other) {
	if(other == NULL || other->isDeleted())
		return false;
	return getRect().intersects(other->getRect());
}

//Get next node in list
Node *Node::getNext() {
	return next;
}

//Add new node after this
void Node::addNode(Node *node) {
	if(next == NULL)
		next = node;
	else
		next->addNode(node);
}

//Remove node immdiately after this from list
void Node::deleteNext() {
	if(next != NULL && next->isDeleted())
		next = next->getNext();
}