#include "Node.h"

/*
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

//Get layer node is attached to
int Node::getLayer() {
	return layer;
}

//Get scaled size of node
sf::Vector2i Node::getSize() {
	sf::Vector2f scale = getGScale();
	return sf::Vector2i(size.x * std::abs(scale.x), size.y * std::abs(scale.y));
}

//Create full collision box
sf::FloatRect Node::getRect() {
	sf::Vector2f pos = this->getGPosition();
	sf::FloatRect rec;
	rec.left = pos.x - (this->getOrigin().x * std::abs(this->getScale().x));
	rec.top = pos.y - (this->getOrigin().y * std::abs(this->getScale().y));
	rec.width = this->getSize().x;
	rec.height = this->getSize().y;
	return rec;
}

//Get parent node
Node *Node::getParent() {
	return parent;
}

//Get global position
sf::Vector2f Node::getGPosition() {
	if(parent != NULL)
		return getPosition() * parent->getScale() + parent->getGPosition();
	return getPosition();
}

sf::Vector2f Node::getGScale() {
	if(parent != NULL)
		return parent->getGScale() * getScale();
	return getScale();
}

sf::Vector2f Node::getInverseScale() {
	return sf::Vector2f(1 / getScale().x, 1 / getScale().y);
}

sf::Vector2f Node::getInverseGScale() {
	return sf::Vector2f(1 / getGScale().x, 1 / getGScale().y);
}

sf::Transform Node::getGTransform() {
	if(parent != NULL)
		return parent->getGTransform().combine(getTransform());
	return getTransform();
}

//Get blend mode for rendering
sf::BlendMode Node::getBlendMode() {
	return blendMode;
}

//Check if node is hidden
bool Node::isHidden() {
	return hidden || deleted || (parent != NULL && parent->isHidden());
}

//Set collision box size
void Node::setSize(sf::Vector2i size) {
	this->size = size;
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

//Set position in global coordinates
void Node::setGPosition(sf::Vector2f pos) {
	if(parent != NULL)
		pos -= parent->getGPosition();
	setPosition(pos);
}

void Node::setGPosition(float x, float y) {
	setGPosition(sf::Vector2f(x, y));
}

//Set blend mode to use in rendering
void Node::setBlendMode(sf::BlendMode blendMode) {
	this->blendMode = blendMode;
}

//Get list of layers node collides with
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

//Create a vector with fixed length in any direction
sf::Vector2f vectorLength(sf::Vector2f dir, double distance) {
	float xOffset = 0;
	float yOffset = 0;
	if(dir.x == 0 && dir.y == 0)
		return sf::Vector2f(0, 0);
	if(dir.x == 0)
		yOffset = copysign(distance, dir.y);
	else if(dir.y == 0)
		xOffset = copysign(distance, dir.x);
	else if(abs(dir.x) == abs(dir.y)) {
		xOffset = RT2O2 * copysign(distance, dir.x);
		yOffset = RT2O2 * copysign(distance, dir.y);
	} else {
		float angle = std::atan2(dir.y, dir.x);
		xOffset = cos(angle) * distance;
		yOffset = sin(angle) * distance;
	}

	return sf::Vector2f(xOffset, yOffset);
}

//Get length of a vector or distance between points
float distance(sf::Vector2f start, sf::Vector2f end) {
	return std::sqrt(std::pow(end.x - start.x, 2) + std::pow(end.y - start.y, 2));
}

sf::Vector2f operator*(const sf::Vector2f &first, const sf::Vector2f &second) {
	return sf::Vector2f(first.x * second.x, first.y * second.y);
}
sf::Vector2f operator*(const sf::Vector2f &first, const sf::Vector2i &second) {
	return sf::Vector2f(first.x * second.x, first.y * second.y);
}
sf::Vector2i operator*(const sf::Vector2i &first, const sf::Vector2i &second) {
	return sf::Vector2i(first.x * second.x, first.y * second.y);
}
sf::Vector2f operator/(const sf::Vector2f &first, const sf::Vector2f &second) {
	return sf::Vector2f(first.x / second.x, first.y / second.y);
}

std::ostream& operator<<(std::ostream& os, const sf::Vector2f &pos) {
	return os << "(" << std::to_string(pos.x) << "," << std::to_string(pos.y) << ") ";
}

std::ostream& operator<<(std::ostream& os, const sf::Vector2i &pos) {
	return os << "(" << std::to_string(pos.x) << "," << std::to_string(pos.y) << ") ";
}