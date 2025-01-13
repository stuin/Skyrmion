#pragma once

#include <bitset>
#include <functional>
#include <stdexcept>

#include "Vector.h"
#include "Event.h"

#define MAXLAYER 16
#define LAYERERROR "Used collision layer > " + MAXLAYER

using Layer = unsigned char;
using sint = long unsigned int;

/*
 * Sprite with collision, events, parenting, etc.
 */

class Node {
private:
	//Base semi-public variables
	Layer layer;
	Node *parent = NULL;
	bool hidden = false;

	//Collision
	sf::Vector2i size;
	std::bitset<MAXLAYER> collisionLayers;

	//Rendering
	sf::Vector2f position = sf::Vector2f(0,0);
	sf::Vector2f scale = sf::Vector2f(1,1);
	sf::Vector2f origin = sf::Vector2f(0,0);
	int blendMode;
	int textureChannel = -1;
	sf::IntRect textureRect;

	//Background system variables
	bool deleted = false;
	Node *next = NULL;

public:

	//Node constructors
	Node(Layer layer=0,
		sf::Vector2i size = sf::Vector2i(16, 16),
		bool hidden = false,
		Node *parent = NULL);

	//General getters
	int getLayer();
	Node *getParent();
	bool isHidden();
	sf::Vector2i getSize();
	sf::FloatRect getRect();

	sf::Vector2f getPosition();
	sf::Vector2f getGPosition();
	sf::Vector2f getScale();
	sf::Vector2f getGScale();
	sf::Vector2f getInverseScale();
	sf::Vector2f getInverseGScale();
	sf::Vector2f getOrigin();
	sf::FloatRect getDrawRect();
	int getBlendMode();
	int getTexture();

	//General setters
	void setParent(Node *parent);
	void setHidden(bool hidden=true);
	void setSize(sf::Vector2i size);

	void setPosition(sf::Vector2f pos);
	void setPosition(float x, float y);
	void setGPosition(sf::Vector2f pos);
	void setGPosition(float x, float y);
	void setScale(sf::Vector2f scale);
	void setOrigin(sf::Vector2f origin);
	void setOrigin(float x, float y);
	void setBlendMode(int blendMode);
	void setTexture(int textureChannel);
	void setTextureRect(sf::IntRect &rectangle);

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

// Use SFML drawable instead of texture
/*class DrawNode : public Node {
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
};*/

//Handles simple animations with horizontal spritesheets
/*class AnimatedNode : public Node {
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

    void update(double time) {
        updateAnimation(time);
    }

    //Update timer
    void updateAnimation(double time) {
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
};*/