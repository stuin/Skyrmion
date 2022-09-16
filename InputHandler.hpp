#include "UpdateList.h"
#include "Settings.h"

class InputHandler : public Node {
public:
	int *controls;
	std::vector<bool> pressed;
	std::vector<bool> held;
	std::function<void(int)> pressedFunc = NULL;
	std::function<void(int)> heldFunc = NULL;

	InputHandler(int *_controls, int count, int layer, Node *parent = NULL) 
	: Node(layer, sf::Vector2i(16, 16), true, parent), controls(_controls), pressed(count, false) {
		pressed.assign(count, false);
		held.assign(count, false);

		UpdateList::addNode(this);
		UpdateList::addListener(this, sf::Event::KeyPressed);
		UpdateList::addListener(this, sf::Event::KeyReleased);
	}

	InputHandler(std::vector<std::string> keys, int layer, Node *parent = NULL)
	: InputHandler(Settings::getControls(keys), keys.size(), layer, parent) {

	}

	void recieveEvent(sf::Event event, WindowSize *windowSize) {
		bool press = (event.type == sf::Event::KeyPressed);
		for(sint i = 0; i < pressed.size(); i++)
			if(event.key.code == controls[i]) {
				pressed[i] = press;
				held[i] = press;
			}
	}

	void clearPressed() {
		pressed.assign(pressed.size(), false);
	}

	void update(double time) {
		for(sint i = 0; i < held.size(); i++) {
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
	std::vector<std::string> listKeys(std::string field) {
		std::vector<std::string> keys = {
			field + "/up",
			field + "/down",
			field + "/left",
			field + "/right",
			field + "/up2",
			field + "/down2",
			field + "/left2",
			field + "/right2"
		};
		return keys;
	}

	DirectionHandler(int *_controls, int count, int layer, Node *parent = NULL)
	: InputHandler(_controls, count, layer, parent) {

	}

	DirectionHandler(std::vector<std::string> keys, int layer, Node *parent = NULL)
	: InputHandler(keys, layer, parent) {

	}

	DirectionHandler(std::string field, int layer, Node *parent = NULL)
	: InputHandler(listKeys(field), layer, parent) {

	}

	void update(double time) {
		sf::Vector2f _direction = sf::Vector2f(0, 0);
		for(sint i = 0; i < held.size(); i++) {
			if(held[i]) {
				//Update direction
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

				//Run lambda functions
				if(heldFunc != NULL)
					heldFunc(i);
				if(pressed[i] && pressedFunc != NULL)
					pressedFunc(i);
			}
		}
		direction = _direction;
		clearPressed();
	}

	sf::Vector2f getDirection() {
		return direction;
	}

	sf::Vector2f getMovement(Node *node, double distance) {
		return node->getShiftedPosition(direction, distance);
	}
};
