#pragma once

#include "Node.h"

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

class ButtonNode : public DrawNode {
public:
	void (*func)() = NULL;

public:
	ButtonNode(void (*func)(), sf::Drawable &image, Layer layer, sf::Vector2i size, Node *parent=NULL) : DrawNode(image, layer, size, parent) {
		this->func = func;
		UpdateList::addListener(this, sf::Event::MouseButtonPressed);
	}

	void recieveEvent(sf::Event event, int shiftX, int shiftY) {
		sf::Vector2i pos(event.mouseButton.x * shiftX, event.mouseButton.y * shiftY);
		if(event.mouseButton.button == sf::Mouse::Left && getRect().contains(pos)) {
			click();
			if(func != NULL)
				func();
		}
	}

	virtual void click() {}
};