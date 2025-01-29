#pragma once

#include <bitset>
#include <functional>
#include <stdexcept>
#include <vector>

#include "Vector.h"
#include "Event.h"

#define MAXLAYER 16
#define LAYERERROR "Used collision layer > " + std::to_string(MAXLAYER)

using Layer = unsigned char;

/*
 * Sprite with collision, events, parenting, etc.
 */

class Node {
private:
	//Base semi-public variables
	sint id;
	Layer layer;
	Node *parent = NULL;
	bool hidden = false;

	//Collision
	Vector2i size;
	std::bitset<MAXLAYER> collisionLayers;

	//Positioning
	Vector2f position = Vector2f(0,0);
	Vector2f scale = Vector2f(1,1);
	Vector2f origin = Vector2f(0,0);

	//Rendering
	int blendMode = 1;
	sint texture = 0;
	bool dirty = false;
	std::vector<TextureRect> textureRects;

	//Background system variables
	bool deleted = false;
	Node *next = NULL;

public:

	//Node constructors
	Node(Layer layer=0,
		Vector2i size = Vector2i(16, 16),
		bool hidden = false,
		Node *parent = NULL);

	//General getters
	sint getId();
	int getLayer();
	Node *getParent();
	bool isHidden();
	Vector2i getSize();
	FloatRect getRect();

	//Positioning
	Vector2f getPosition();
	Vector2f getGPosition();
	Vector2f getScale();
	Vector2f getGScale();
	Vector2f getInverseScale();
	Vector2f getInverseGScale();
	Vector2f getOrigin();

	//Rendering
	FloatRect getDrawRect();
	int getBlendMode();
	int getTexture();
	bool isDirty();
	virtual std::vector<TextureRect> *getTextureRects();

	//General setters
	void setParent(Node *parent);
	void setHidden(bool hidden=true);
	void setSize(Vector2i size);

	//Positioning
	void setPosition(Vector2f pos);
	void setPosition(float x, float y);
	void setGPosition(Vector2f pos);
	void setGPosition(float x, float y);
	void setSPosition(Vector2f pos);
	void setSPosition(float x, float y);
	void setScale(Vector2f scale);
	void setScale(float x, float y);
	void setOrigin(Vector2f origin);
	void setOrigin(float x, float y);

	//Rendering
	void setBlendMode(int blendMode);
	void setTexture(int textureChannel);
	void setDirty(bool dirty=true);
	void setTextureRect(TextureRect rectangle, sint i=0);
	void createPixelRect(FloatRect rect, Vector2i pixel, sint i=0);

	//Collision system
	std::bitset<MAXLAYER> getCollisionLayers();
	bool getCollisionLayer(Layer layer);
	void collideWith(Layer layer, bool collide=true);

	//Linked list functions
	Node *getNext();
	void addNode(Node *node);
	void deleteNext();

	//Proper deletion procedure
	bool isDeleted() {
		return deleted;
	}
	void setDelete() {
		deleted = true;
	}
	virtual ~Node() {}

	//Entity implementation
	virtual void update(double time) {}
	virtual void collide(Node *object) {}
	virtual void collide(Node *object, double time) {
		collide(object);
	}
	virtual void recieveEvent(Event event) {}
	virtual void recieveSignal(int id, Node *sender) {}
	virtual void reloadBuffer() {}
};