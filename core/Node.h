#pragma once

#include <bitset>

#include "Event.h"
#include "RenderComponent.h"

#define MAXLAYER 16
#define LAYERERROR "Used layer > " + std::to_string(MAXLAYER)
#define DRAWLAYERERROR "Used node layer < 0"

/*
 * UNode has id, updating, and events
 * Node adds transform, collision, parenting, etc.
 */

class UNode {
private:
	sint id;
	int layer;

	//Background system variables
	bool deleted = false;
	UNode *next = NULL;

public:
	//Node constructors
	UNode(int layer=0);

	//General getters
	sint getId();
	int getLayer();

	//Linked list functions
	UNode *getNext();
	void addNode(UNode *node);
	void deleteNext();

	//Proper deletion procedure
	bool isDeleted() {
		return deleted;
	}
	void setDelete() {
		deleted = true;
	}
	virtual ~UNode() {}

	//Custom functions
	virtual void update(double time) {}
	virtual void recieveEvent(Event event) {}
	virtual void recieveSignal(int id, UNode *sender) {}
};

class Node : public UNode {
private:
	//Base semi-public variables
	Node *parent = NULL;
	bool hidden = false;

	//Collision
	Vector2i size = Vector2i(1,1);
	std::bitset<MAXLAYER> collisionLayers;

	//Positioning
	Vector2f position = Vector2f(0.0f,0.0f);
	Vector2f scale = Vector2f(1.0f,1.0f);
	Vector2f origin = Vector2f(0.0f,0.0f);

	RenderComponent *rendering = NULL;

public:

	//Node constructors
	Node(int layer=0,
		int renderType=RENDER_SINGLE_TEXTURE,
		Vector2i size = Vector2i(16, 16),
		bool hidden = false,
		Node *parent = NULL);

	//General getters
	Node *getParent();
	bool isHidden();
	Vector2f getSize();
	FloatRect getRect();

	//Positioning
	Vector2f getPosition();
	Vector2f getGPosition();
	Vector2f getScale();
	Vector2f getOrigin();
	Vector2f getSOrigin();

	//Rendering
	RenderComponent *getRenderComponent();
	int getBlendMode();
	sint getTexture();
	skColor getColor();
	std::vector<TextureRect> *getTextureRects();
	const char *getString();

	//General setters
	void setParent(Node *parent);
	void setHidden(bool hidden=true);
	void setSize(Vector2i size);
	void setSize(int x, int y);

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
	void setRenderComponent(int type);
	void setRenderComponent(RenderComponent *component);
	void setBlendMode(int blendMode);
	void setTexture(sint textureChannel);
	void setColor(skColor color);
	void setTextureRect(TextureRect rectangle, sint i=0);
	void setTextureIntRect(IntRect rect, sint i=0);
	void setTextureVecRect(Vector2i corner, sint i=0);
	void setTextureVecRect(int x, int y, sint i=0);
	void createPixelRect(FloatRect rect, Vector2i pixel, sint i=0);
	void setString(const char *text);

	//Collision system
	std::bitset<MAXLAYER> getCollisionLayers();
	bool getCollisionLayer(int layer);
	void collideWith(int layer, bool collide=true);

	//Custom functions
	virtual ~Node() {}
	virtual void collide(Node *object) {}
	virtual void collide(Node *object, double time) {
		collide(object);
	}
	virtual void reloadBuffer() {}
};