#pragma once

#include "../core/Node.h"

#include <vector>

/*
 * Render multiple shapes and texts
 */

class Canvas : public Node {
private:
	std::vector<sf::Shape *> shapes;
	std::vector<sf::Text> words;

	sf::Color color;
	sf::Font font;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        long unsigned int i;
        for(i = 0; i < shapes.size(); i++)
        	target.draw(*shapes[i], states);
        for(i = 0; i < words.size(); i++)
        	target.draw(words[i], states);
    }

public:
	Canvas(std::string fontFile, sf::Vector2i size, Layer layer, sf::Color _color=sf::Color::Magenta, Node *parent=NULL) :
	Node(layer, size, false, parent) {
		font.loadFromFile(fontFile);
		color = _color;
		setOrigin(0, 0);
	}

	void addText(sf::Vector2f pos, std::string content, int fSize=6) {
		sf::Text text(content, font);
		text.setCharacterSize(fSize);
		text.setFillColor(color);
		text.setPosition(pos);
		words.push_back(text);
	}

	void addDot(sf::Vector2f pos, float cSize=1) {
		sf::CircleShape *circle = new sf::CircleShape(cSize);
		circle->setFillColor(color);
		circle->setPosition(pos);
		circle->setOrigin(sf::Vector2f(cSize, cSize));
		shapes.push_back(circle);
	}

	void addPoint(sf::Vector2f pos, std::string content, int fSize=6, float cSize=1) {
		addText(pos + sf::Vector2f(cSize*2, 0), content, fSize);
		addDot(pos, cSize);
	}

	void clear() {
		words.clear();
		for(sf::Shape *s : shapes)
			delete s;
		shapes.clear();
	}
};