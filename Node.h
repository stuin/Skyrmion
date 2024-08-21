#pragma once

#include <bitset>
#include <functional>
#include <iostream>
#include <math.h>
#include <stdexcept>

#include "GridMaker.h"

#define MAXLAYER 16
#define LAYERERROR "Used collision layer > " + MAXLAYER

#define RT2O2 sqrt(2) / 2.0

using Layer = unsigned char;
using sint = long unsigned int;

/*
 * Created by Stuart Irwin on 4/13/2019.
 * Sprite with collision
 */

struct WindowSize {
	float shiftX;
	float shiftY;
	int cornerX;
	int cornerY;

	sf::Vector2f worldPos(int x, int y) {
		return sf::Vector2f(x * shiftX + cornerX, y * shiftY + cornerY);
	}
};

class Node : public sf::Sprite {
private:
	//Base public variables
	Layer layer;
	Node *parent = NULL;
	bool hidden = false;

	//Collision
	sf::Vector2i size;
	std::bitset<MAXLAYER> collisionLayers;

	//Background system variables
	bool deleted = false;
	Node *next = NULL;

public:
	sf::BlendMode blendMode;

	//Node constructors
	Node(Layer layer=0,
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

	//General setters
	void setSize(sf::Vector2i size);
	void setHidden(bool hidden=true);
	void setParent(Node *parent);
	void setGPosition(sf::Vector2f pos);
	void setGPosition(float x, float y);

	//Collision system
	std::bitset<MAXLAYER> getCollisionLayers();
	bool getCollisionLayer(Layer layer);
	void collideWith(Layer layer, bool collide=true);

	//Other math utilities
	sf::Vector2f move(sf::Vector2f dir, double distance, int collideOffset=0);
	sf::Vector2f move(sf::Vector2f dir, Indexer *indexes, double distance, int collideOffset=0);
	static sf::Vector2f gridCollision(sf::Vector2f start, sf::Vector2f move, Indexer *indexes, int collideOffset);

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
	virtual void recieveSignal(int id, Node *sender) {}
	virtual void reloadBuffer() {}
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

	sf::Drawable *getImage() {
		return image;
	}
};

class AnimatedNode : public Node {
	int frameWidth = 0;
	int frameHeight = 0;
    int maxFrames = 0;
    int frame = 0;
    double nextTime = 0;
    double delay = -1;
    bool paused = false;
public:
	AnimatedNode(sf::Texture &texture, int _maxFrames, double _delay, Layer layer, sf::Vector2i size) : Node(layer, size) {
		setTexture(texture);

		maxFrames = _maxFrames;
		delay = _delay;
		frameWidth = texture.getSize().x / _maxFrames;
		frameHeight = texture.getSize().y;
	}

	//Update timer
    void update(double time) {
        if(!paused) {
            if((nextTime -= time) <= 0) {
                nextTime = delay;
                frame++;

                //Reset to start frame
                if(frame == maxFrames)
                    frame = 0;

                setTextureRect(sf::IntRect(frameWidth * frame, 0, frameWidth, frameHeight));
            }
        }
    }
};

sf::Vector2f vectorLength(sf::Vector2f dir, double distance);
sf::Vector2f operator*(const sf::Vector2f &first, const sf::Vector2f &second);
sf::Vector2f operator/(const sf::Vector2f &first, const sf::Vector2f &second);
float distance(sf::Vector2f start, sf::Vector2f end=sf::Vector2f(0,0));
std::string getString(sf::Vector2f pos);