#include "UpdateList.h"

class InputHandler : public Node {
private:
	sf::Keyboard::Key *controls;
	bool pressed[4] = {false, false, false, false};
	sf::Vector2f direction = sf::Vector2f(0, 0);

public:
	InputHandler(sf::Keyboard::Key *_controls, int layer, Node *parent = NULL) : Node(layer, sf::Vector2i(16, 16), true, parent), controls(_controls) {
		UpdateList::addNode(this);
		UpdateList::addListener(this, sf::Event::KeyPressed);
		UpdateList::addListener(this, sf::Event::KeyReleased);
	}

	void recieveEvent(sf::Event event, int shiftX, int shiftY) {
		bool press = (event.type == sf::Event::KeyPressed);
		for(int i = 0; i < 4; i++)
			if(event.key.code == controls[i])
				pressed[i] = press;
	}

	void update(double time) {
		sf::Vector2f _direction = sf::Vector2f(0, 0);
		if(pressed[0]) // up
			_direction.y--;
		if(pressed[1]) // down
			_direction.y++;
		if(pressed[2]) // left
			_direction.x--;
		if(pressed[3]) // right
			_direction.x++;
		direction = _direction;
	}

	sf::Vector2f getMovement(Node *node, double distance) {
		return node->getShiftedPosition(direction, distance);
	}
};
