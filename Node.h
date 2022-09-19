#pragma once

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <bitset>
#include <stdexcept>
#include <math.h>

#include <iostream>

#define MAXLAYER 16
#define LAYERERROR "Used collision layer > 16"

using Layer = unsigned char;
using sint = long unsigned int;

/*
 * Created by Stuart Irwin on 4/13/2019.
 * Sprite with collision
 */

struct WindowSize {
	int shiftX;
	int shiftY;
	int cornerX;
	int cornerY;

	sf::Vector2f worldPos(int x, int y) {
		return sf::Vector2f(x * shiftX + cornerX, y * shiftY + cornerY);
	}
};

class Node : public sf::Sprite {
private:
	//Visible system variables
	sf::Vector2i size;
	bool hidden = false;
	Node *parent = NULL;
	std::bitset<MAXLAYER> collisionLayers;
	Layer layer;

	//Background system variables
	bool deleted = false;
	Node *next = NULL;

public:
	//Node constructors
	Node(Layer layer,
		sf::Vector2i size = sf::Vector2i(16, 16),
		bool hidden = false,
		Node *parent = NULL);

	//General getters
	int getLayer();
	sf::Vector2i getSize();
	sf::FloatRect getRect();
	bool isHidden();
	Node *getParent();
	sf::Vector2f getGPosition();
	sf::Vector2f getShiftedPosition(sf::Vector2f dir, double distance);

	//General setters
	void setSize(sf::Vector2i size);
	void setHidden(bool hidden=true);
	void setParent(Node *parent);
	void setGPosition(float x, float y);

	//Collision system
	std::bitset<MAXLAYER> getCollisionLayers();
	bool getCollisionLayer(Layer layer);
	void collideWith(Layer layer, bool collide=true);
	bool checkCollision(Node *other);

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
	virtual void recieveEvent(sf::Event event, WindowSize *windowSize) {}
};

class DrawNode : public Node {
private:
	sf::Drawable *image = NULL;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		if(image != NULL) {
			states.transform *= getTransform();
			target.draw(*image, states);
		}
	}

public:
	DrawNode(sf::Drawable &image, Layer layer, sf::Vector2i size=sf::Vector2i(16,16), Node *parent=NULL) : Node(layer, size, false, parent) {
		this->image = &image;
	}

	void setImage(sf::Drawable &image) {
		this->image = &image;
	}
};