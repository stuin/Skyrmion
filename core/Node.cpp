#include "Node.h"
#include "UpdateList.h"

/*
 * Sprite with collision
 */

unsigned int currentId = 0;

//Base constructor
Node::Node(Layer layer, Vector2i size, bool hidden, Node *parent) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	this->layer = layer;
	this->id = currentId++;

	setSize(size);
	setHidden(hidden);
	setParent(parent);
}

sint Node::getId() {
	return id;
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
Vector2i Node::getSize() {
	Vector2f scale = getGScale();
	return (size * scale).abs();
}

//Create full collision box
FloatRect Node::getRect() {
	Vector2f start = this->getGPosition() - origin * getGScale().abs();
	Vector2f end = this->getSize();
	FloatRect rec;
	rec.left = start.x;
	rec.top = start.y;
	rec.width = end.x;
	rec.height = end.y;
	return rec;
}

const char *Node::getString() {
	return text;
}

//Get local position
Vector2f Node::getPosition() {
	return position;
}

//Get global position
Vector2f Node::getGPosition() {
	if(relativeScale && parent != NULL)
		return position * parent->getScale() + parent->getGPosition();
	if(parent != NULL)
		return position + parent->getGPosition();
	return position;
}

//Get local scale
Vector2f Node::getScale() {
	return scale;
}

//Get global scale
Vector2f Node::getGScale() {
	if(relativeScale && parent != NULL)
		return parent->getGScale() * getScale();
	return getScale();
}

//Get inverted scale for children
Vector2f Node::getInverseScale() {
	return Vector2f(1 / getScale().x, 1 / getScale().y);
}

Vector2f Node::getInverseGScale() {
	return Vector2f(1 / getGScale().x, 1 / getGScale().y);
}

//Get texture origin
Vector2f Node::getOrigin() {
	return origin;
}
Vector2f Node::getSOrigin() {
	return origin*getGScale().abs();
}

//Get blend mode for rendering
int Node::getBlendMode() {
	return blendMode;
}

//Get texture number
sint Node::getTexture() {
	return texture;
}

skColor Node::getColor() {
	return color;
}

//Get sections of texture to render
std::vector<TextureRect> *Node::getTextureRects() {
	return &textureRects;
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
void Node::setSize(Vector2i size) {
	this->size = size;
	setOrigin(size.x / 2, size.y / 2);
}

//Set position in local coordinates
void Node::setPosition(Vector2f pos) {
	this->position = pos;
}
void Node::setPosition(float x, float y) {
	this->position = Vector2f(x, y);
}

//Set position in global coordinates
void Node::setGPosition(Vector2f pos) {
	if(parent != NULL)
		pos = (pos - parent->getGPosition()) / parent->getScale();
	setPosition(pos);
}
void Node::setGPosition(float x, float y) {
	setGPosition(Vector2f(x, y));
}

//Set position in unscaled coordinates
void Node::setUPosition(Vector2f pos) {
	if(parent != NULL)
		pos /= parent->getScale();
	setPosition(pos);
}
void Node::setUPosition(float x, float y) {
	setUPosition(Vector2f(x, y));
}

//Set position in screen coordinates
void Node::setSPosition(Vector2f pos) {
	setGPosition(screenToGlobal(pos.x, pos.y));
}
void Node::setSPosition(float x, float y) {
	setGPosition(screenToGlobal(x, y));
}

//Set scale
void Node::setScale(Vector2f scale, bool relative) {
	this->scale = scale;
	this->relativeScale = relative;
}
void Node::setScale(float x, float y, bool relative) {
	this->scale = Vector2f(x, y);
	this->relativeScale = relative;
}

//Set origin
void Node::setOrigin(Vector2f origin) {
	this->origin = origin;
}
void Node::setOrigin(float x, float y) {
	this->origin = Vector2f(x, y);
}

//Set blend mode to use in rendering
void Node::setBlendMode(int blendMode) {
	this->blendMode = blendMode;
}

//Set texture channel
void Node::setTexture(sint texture) {
	this->texture = texture;
}

void Node::setColor(skColor color) {
	this->color = color;
}

//Set texture rect
void Node::setTextureRect(TextureRect rectangle, sint i) {
	while(i >= textureRects.size())
		textureRects.push_back({0});
	textureRects[i] = rectangle;
}

//Set basic texture rect
void Node::setTextureIntRect(IntRect rect, sint i) {
	while(i >= textureRects.size())
		textureRects.push_back({0});
	textureRects[i] = {0, 0, (float)rect.width,(float)rect.height, rect.left,rect.top, rect.width,rect.height, 0};
}

//Set texture rect based on corner and node size
void Node::setTextureVecRect(Vector2i corner, sint i) {
	setTextureVecRect(corner.x, corner.y, i);
}
void Node::setTextureVecRect(int x, int y, sint i) {
	setTextureIntRect(IntRect(x, y, size.x, size.y));
}

//Create rectangle borders from one pixel of texture
void Node::createPixelRect(FloatRect rect, Vector2i pixel, sint i) {
	setTextureRect({rect.left,rect.top, 			rect.width,1,  pixel.x,pixel.y, 1,1,0}, i+0);
	setTextureRect({rect.left,rect.top+rect.height,	rect.width,1,  pixel.x,pixel.y, 1,1,0}, i+1);
	setTextureRect({rect.left,rect.top, 			1,rect.height, pixel.x,pixel.y, 1,1,0}, i+2);
	setTextureRect({rect.left+rect.width,rect.top, 	1,rect.height, pixel.x,pixel.y, 1,1,0}, i+3);
}

void Node::setString(const char *text) {
	this->text = text;
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

//Convert screen space coordinates to global
Vector2f screenToGlobal(float x, float y) {
	Vector2f pos = Vector2f(x,y);
	if(pos.x < 0)
		pos.x += UpdateList::getScreenRect().left;
	if(pos.y < 0)
		pos.y += UpdateList::getScreenRect().top;
	pos *= UpdateList::getScaleFactor();
	pos += UpdateList::getCameraRect().pos();
	return pos;
}