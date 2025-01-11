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

//Get parent node
Node *Node::getParent() {
	return parent;
}

//Check if node is hidden
bool Node::isHidden() {
	return hidden || deleted || (parent != NULL && parent->isHidden());
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

//Get local position
sf::Vector2f Node::getPosition() {
	return position;
}

//Get global position
sf::Vector2f Node::getGPosition() {
	if(parent != NULL)
		return position * parent->getScale() + parent->getGPosition();
	return position;
}

//Get local scale
sf::Vector2f Node::getScale() {
	return scale;
}

//Get global scale
sf::Vector2f Node::getGScale() {
	if(parent != NULL)
		return parent->getGScale() * getScale();
	return getScale();
}

//Get inverted scale for children
sf::Vector2f Node::getInverseScale() {
	return sf::Vector2f(1 / getScale().x, 1 / getScale().y);
}

sf::Vector2f Node::getInverseGScale() {
	return sf::Vector2f(1 / getGScale().x, 1 / getGScale().y);
}

//Get texture origin
sf::Vector2f Node::getOrigin() {
	return origin;
}

/*sf::Transform Node::getGTransform() {
	if(parent != NULL)
		return parent->getGTransform().combine(getTransform());
	return getTransform();
}*/

sf::FloatRect Node::getDrawRect() {
	return getRect();
}

//Get blend mode for rendering
int Node::getBlendMode() {
	return blendMode;
}

//Get texture number
int Node::getTexture() {
	return textureChannel;
}

//Set parent node
void Node::setParent(Node *parent) {
	this->parent = parent;
}

//Set whether node is hidden
void Node::setHidden(bool hidden) {
	this->hidden = hidden;
}

//Set collision box size
void Node::setSize(sf::Vector2i size) {
	this->size = size;
	setOrigin(size.x / 2, size.y / 2);
}

//Set position in local coordinates
void Node::setPosition(sf::Vector2f pos) {
	this->position = pos;
}
void Node::setPosition(float x, float y) {
	this->position = sf::Vector2f(x, y);
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

//Set scale
void Node::setScale(sf::Vector2f scale) {
	this->scale = scale;
}

//Set origin
void Node::setOrigin(sf::Vector2f origin) {
	this->origin = origin;
}
void Node::setOrigin(float x, float y) {
	this->origin = sf::Vector2f(x, y);
}

//Set blend mode to use in rendering
void Node::setBlendMode(int blendMode) {
	this->blendMode = blendMode;
}

//Set texture channel
void Node::setTexture(int texture) {
	this->textureChannel = texture;
}

//Set texture rect
void Node::setTextureRect(sf::IntRect &rectangle) {
	this->textureRect = rectangle;
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