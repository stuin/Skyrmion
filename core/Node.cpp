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
	rendering = createRenderComponent(renderType, this);

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
	if(rendering != NULL)
		return rendering->isHidden() || isDeleted() || (parent != NULL && parent->isHidden());
	return true;
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

RenderComponent *Node::getRenderComponent(bool passBuffer) {
	if(passBuffer && rendering != NULL && rendering->getType() == RENDER_PASSTHROUGH_BUFFER)
		return rendering->getSubComponent();
	return rendering;
}

//Get blend mode for rendering
int Node::getBlendMode() {
	if(rendering != NULL)
		return getRenderComponent()->getBlendMode();
	else
		throw new RENDERCOMPONENTNULL;
}

//Get texture number
sint Node::getTexture() {
	if(rendering != NULL)
		return getRenderComponent()->getTexture();
	else
		throw new RENDERCOMPONENTNULL;
}

Vector2i Node::getTextureSize() {
	return UpdateList::getTextureSize(getTexture());
}

skColor Node::getColor() {
	if(rendering != NULL)
		return getRenderComponent()->getColor();
	else
		throw new RENDERCOMPONENTNULL;
}

//Get sections of texture to render
std::vector<TextureRect> *Node::getTextureRects() {
	if(rendering != NULL)
		return getRenderComponent()->getTextureRects();
	else
		throw new RENDERCOMPONENTNULL;
}

const char *Node::getString() {
	if(rendering != NULL)
		return getRenderComponent()->getString();
	else
		throw new RENDERCOMPONENTNULL;
}

//Set parent node
void Node::setParent(Node *_parent) {
	this->parent = _parent;
}

//Set whether node is hidden
void Node::setHidden(bool _hidden) {
	if(rendering != NULL)
		rendering->setHidden(_hidden);
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
	rendering = createRenderComponent(type, this);
}
//void Node::setRenderComponent(RenderComponent *component) {
//	if(rendering != NULL)
//		delete rendering;
//	rendering = component;
//}

//Pass data to RenderComponent

//Set blend mode to use in rendering
void Node::setBlendMode(int _blendMode) {
	if(rendering != NULL)
		getRenderComponent()->setBlendMode(_blendMode);
	else
		throw new RENDERCOMPONENTNULL;
}

//Set texture channel
void Node::setTexture(sint _texture) {
	if(rendering != NULL)
		getRenderComponent()->setTexture(_texture);
	else
		throw new RENDERCOMPONENTNULL;
}

void Node::setColor(skColor _color) {
	if(rendering != NULL)
		getRenderComponent()->setColor(_color);
	else
		throw new RENDERCOMPONENTNULL;
}

//Set texture rect
void Node::setTextureRect(TextureRect rectangle, sint i) {
	if(rendering != NULL)
		getRenderComponent()->setTextureRect(rectangle, i);
	else
		throw new RENDERCOMPONENTNULL;
}

//Set basic texture rect
void Node::setTextureIntRect(IntRect rect, sint i) {
	if(rendering != NULL)
		getRenderComponent()->setTextureIntRect(rect, i);
	else
		throw new RENDERCOMPONENTNULL;
}

//Set texture rect based on corner and node size
void Node::setTextureVecRect(Vector2i corner, sint i) {
	if(rendering != NULL)
		getRenderComponent()->setTextureVecRect(corner, size, i);
	else
		throw new RENDERCOMPONENTNULL;
}
void Node::setTextureVecRect(int x, int y, sint i) {
	if(rendering != NULL)
		getRenderComponent()->setTextureVecRect(Vector2i(x, y), size, i);
	else
		throw new RENDERCOMPONENTNULL;
}

//Create rectangle borders from one pixel of texture
void Node::createPixelRect(FloatRect rect, Vector2i pixel, sint i) {
	if(rendering != NULL)
		getRenderComponent()->createPixelRect(rect, pixel, i);
	else
		throw new RENDERCOMPONENTNULL;
}

void Node::setString(const char *_text) {
	if(rendering != NULL)
		getRenderComponent()->setString(_text);
	else
		throw new RENDERCOMPONENTNULL;
}

//Setup RENDER_PASSTHROUGH_BUFFER Component and Buffer Texture
void Node::setupBuffer(skColor _color) {
	RenderComponent *buffer = createRenderComponent(RENDER_PASSTHROUGH_BUFFER, this);
	buffer->setSubComponent(rendering);
	rendering = buffer;
	buffer->setTexture(UpdateList::createBuffer(this, _color));
}

void Node::scheduleBufferRefresh(sint buffer) {
	if(buffer != 0)
		UpdateList::scheduleBufferRefresh(buffer);
	else if(rendering != NULL && rendering->getType() == RENDER_PASSTHROUGH_BUFFER)
		UpdateList::scheduleBufferRefresh(rendering->getTexture());
	else
		throw new RENDERCOMPONENTNULL;
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