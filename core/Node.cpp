#include "Node.h"
#include "UpdateList.h"

/*
 * UNode and Node implementations
 */

unsigned int currentId = 0;

//Base constructor
UNode::UNode(int layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	this->layer = layer;
	this->id = currentId++;
}

sint UNode::getId() {
	return id;
}

//Get layer node is attached to
int UNode::getLayer() {
	return layer;
}

//Get next node in list
UNode *UNode::getNext() {
	return next;
}

//Add new node after this
void UNode::addNode(UNode *node) {
	if(next == NULL)
		next = node;
	else
		next->addNode(node);
}

//Remove node immdiately after this from list
void UNode::deleteNext() {
	if(next != NULL && next->isDeleted())
		next = next->getNext();
}

//Base constructor
Node::Node(int layer, int renderType, Vector2i size, bool hidden, Node *parent) : UNode(layer) {
	if(layer < 0)
		throw new std::invalid_argument(DRAWLAYERERROR);
	rendering = createRenderComponent(renderType);

	setSize(size);
	setOrigin(size.x / 2, size.y / 2);
	setHidden(hidden);
	setParent(parent);
}

//Get parent node
Node *Node::getParent() {
	return parent;
}

//Check if node is hidden
bool Node::isHidden() {
	return hidden || isDeleted() || (parent != NULL && parent->isHidden());
}

//Get scaled size of node
Vector2f Node::getSize() {
	return (scale * size).abs();
}

//Create full collision box
FloatRect Node::getRect() {
	Vector2f start = this->getGPosition() - origin * getScale().abs();
	Vector2f end = this->getSize();
	FloatRect rec;
	rec.left = start.x;
	rec.top = start.y;
	rec.width = end.x;
	rec.height = end.y;
	return rec;
}

//Get local position
Vector2f Node::getPosition() {
	return position;
}

//Get global position
Vector2f Node::getGPosition() {
	if(parent != NULL)
		return position + parent->getGPosition();
	return position;
}

//Get scale
Vector2f Node::getScale() {
	return scale;
}

//Get texture origin
Vector2f Node::getOrigin() {
	return origin;
}
Vector2f Node::getSOrigin() {
	return origin*getScale().abs();
}

RenderComponent *Node::getRenderComponent() {
	return rendering;
}

//Get blend mode for rendering
int Node::getBlendMode() {
	if(rendering != NULL)
		return rendering->getBlendMode();
	else
		return 1;
}

//Get texture number
sint Node::getTexture() {
	if(rendering != NULL)
		return rendering->getTexture();
	else
		return 0;
}

skColor Node::getColor() {
	if(rendering != NULL)
		return rendering->getColor();
	else
		return COLOR_WHITE;
}

//Get sections of texture to render
std::vector<TextureRect> *Node::getTextureRects() {
	if(rendering != NULL)
		return rendering->getTextureRects();
	else
		return NULL;
}

const char *Node::getString() {
	if(rendering != NULL)
		return rendering->getString();
	else
		return NULL;
}

//Set parent node
void Node::setParent(Node *_parent) {
	this->parent = _parent;
}

//Set whether node is hidden
void Node::setHidden(bool _hidden) {
	this->hidden = _hidden;
}

//Set collision box size
void Node::setSize(Vector2i _size) {
	Vector2f o = this->origin / this->size;
	this->size = _size;
	setOrigin(size * o);
}
void Node::setSize(int x, int y) {
	setSize(Vector2i(x, y));
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
		pos = (pos - parent->getGPosition());
	setPosition(pos);
}
void Node::setGPosition(float x, float y) {
	setGPosition(Vector2f(x, y));
}

//Set position in screen coordinates
void Node::setSPosition(Vector2f pos) {
	setGPosition(screenToGlobal(pos.x, pos.y));
}
void Node::setSPosition(float x, float y) {
	setGPosition(screenToGlobal(x, y));
}

//Set scale
void Node::setScale(Vector2f _scale) {
	this->scale = _scale;
}
void Node::setScale(float x, float y) {
	this->scale = Vector2f(x, y);
}

//Set origin
void Node::setOrigin(Vector2f _origin) {
	this->origin = _origin;
}
void Node::setOrigin(float x, float y) {
	this->origin = Vector2f(x, y);
}

//Link rendering component
void Node::setRenderComponent(int type) {
	if(rendering != NULL)
		delete rendering;
	rendering = createRenderComponent(type);
}
void Node::setRenderComponent(RenderComponent *component) {
	if(rendering != NULL)
		delete rendering;
	rendering = component;
}

//Pass data to RenderComponent

//Set blend mode to use in rendering
void Node::setBlendMode(int _blendMode) {
	if(rendering != NULL)
		rendering->setBlendMode(_blendMode);
}

//Set texture channel
void Node::setTexture(sint _texture) {
	if(rendering != NULL)
		rendering->setTexture(_texture);
}

void Node::setColor(skColor _color) {
	if(rendering != NULL)
		rendering->setColor(_color);
}

//Set texture rect
void Node::setTextureRect(TextureRect rectangle, sint i) {
	if(rendering != NULL)
		rendering->setTextureRect(rectangle, i);
}

//Set basic texture rect
void Node::setTextureIntRect(IntRect rect, sint i) {
	if(rendering != NULL)
		rendering->setTextureIntRect(rect, i);
}

//Set texture rect based on corner and node size
void Node::setTextureVecRect(Vector2i corner, sint i) {
	if(rendering != NULL)
		rendering->setTextureVecRect(corner, size, i);
}
void Node::setTextureVecRect(int x, int y, sint i) {
	if(rendering != NULL)
		rendering->setTextureVecRect(Vector2i(x, y), size, i);
}

//Create rectangle borders from one pixel of texture
void Node::createPixelRect(FloatRect rect, Vector2i pixel, sint i) {
	if(rendering != NULL)
		rendering->createPixelRect(rect, pixel, i);
}

void Node::setString(const char *_text) {
	if(rendering != NULL)
		rendering->setString(_text);
}

//Get list of layers node collides with
std::bitset<MAXLAYER> Node::getCollisionLayers() {
	return collisionLayers;
}

//Check if node collides with layer
bool Node::getCollisionLayer(int layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return collisionLayers[layer];
}

//Set if node collides with layer
void Node::collideWith(int layer, bool collide) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	collisionLayers[layer] = collide;
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