#include "UpdateList.h"

class InputHandler : public Node {
public:
	sf::Keyboard::Key *controls;
	std::vector<bool> pressed;
	std::vector<bool> held;
	std::function<void(int)> pressedFunc = NULL;
	std::function<void(int)> heldFunc = NULL;

	InputHandler(sf::Keyboard::Key *_controls, int count, int layer, Node *parent = NULL) 
	: Node(layer, sf::Vector2i(16, 16), true, parent), controls(_controls), pressed(count, false) {
		pressed.assign(count, false);
		held.assign(count, false);

		UpdateList::addNode(this);
		UpdateList::addListener(this, sf::Event::KeyPressed);
		UpdateList::addListener(this, sf::Event::KeyReleased);
	}

	void recieveEvent(sf::Event event, WindowSize *windowSize) {
		bool press = (event.type == sf::Event::KeyPressed);
		for(int i = 0; i < 4; i++)
			if(event.key.code == controls[i]) {
				pressed[i] = press;
				held[i] = press;
			}
	}

	void clearPressed() {
		pressed.assign(pressed.size(), false);
	}

	void update(double time) {
		for(int i = 0; i < pressed.size(); i++) {
			if(held[i]) {
				if(heldFunc != NULL)
					heldFunc(i);
				if(pressed[i] && pressedFunc != NULL)
					pressedFunc(i);
			}
		}

		clearPressed();
	}
};

class DirectionHandler : public InputHandler {
private:
	sf::Vector2f direction = sf::Vector2f(0, 0);

public:
	DirectionHandler(sf::Keyboard::Key *_controls, int layer, Node *parent = NULL)
	: InputHandler(_controls, 4, layer, parent) {

	}

	void update(double time) {
		sf::Vector2f _direction = sf::Vector2f(0, 0);
		for(int i = 0; i < held.size(); i++) {
			if(held[i]) {
				switch(i % 4) {
					case 0: // up
						_direction.y--;
						break;
					case 1: // down
						_direction.y++;
						break;
					case 2: // left
						_direction.x--;
						break;
					case 3: // right
						_direction.x++;
						break;
				}
			}
		}
		direction = _direction;
	}

	sf::Vector2f getMovement(Node *node, double distance) {
		return node->getShiftedPosition(direction, distance);
	}
};
