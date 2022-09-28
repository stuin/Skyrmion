#include <cstdlib>

#include "UpdateList.h"
#include "Settings.h"

#define MAXALTS 2
#define JOYSTICK_ZONE 5.0f

class InputHandler : public Node {
private:
	void add_listeners() {
		UpdateList::addNode(this);
		UpdateList::addListener(this, sf::Event::KeyPressed);
		UpdateList::addListener(this, sf::Event::KeyReleased);
		UpdateList::addListener(this, sf::Event::MouseButtonPressed);
		UpdateList::addListener(this, sf::Event::MouseButtonReleased);
		UpdateList::addListener(this, sf::Event::MouseWheelScrolled);
		UpdateList::addListener(this, sf::Event::JoystickButtonPressed);
		UpdateList::addListener(this, sf::Event::JoystickButtonReleased);
		UpdateList::addListener(this, sf::Event::JoystickMoved);
	}

public:
	std::vector<int> controls;
	std::vector<std::string> keycodes;
	std::vector<bool> pressed;
	std::vector<bool> held;
	std::function<void(int)> pressedFunc = NULL;
	std::function<void(int)> heldFunc = NULL;

	int remap = -1;
	int count = 0;

	InputHandler(std::vector<int> _controls, int layer, Node *parent = NULL) 
	: Node(layer, sf::Vector2i(16, 16), true, parent), controls(_controls) {
		keycodes.assign(_controls.size(), "");
		pressed.assign(_controls.size(), false);
		held.assign(_controls.size(), false);
		count = _controls.size();

		add_listeners();
	}

	InputHandler(std::vector<std::string> keys, int layer, Node *parent = NULL)
	: Node(layer, sf::Vector2i(16, 16), true, parent) {
		pressed.assign(keys.size() * MAXALTS, false);
		held.assign(keys.size() * MAXALTS, false);
		count = keys.size();

		//Base keys
		for(std::string s : keys) {
			keycodes.push_back(s);
			controls.push_back(Settings::getControl(s));
		}

		//Alternate keys
		for(int i = 1; i < MAXALTS; i++) {
			for(std::string s : keys) {
				keycodes.push_back(s + "&" + std::to_string(i));
				controls.push_back(Settings::getControl(s + "&" + std::to_string(i)));
			}
		}

		add_listeners();
	}

	int addKey(int code) {
		controls.insert(controls.begin() + count, code);
		keycodes.insert(keycodes.begin() + count, "");
		pressed.insert(pressed.begin() + count, false);
		held.insert(held.begin() + count, false);
		return count++;
	}

	int addKey(std::string key) {
		int i = addKey(Settings::getControl(key));
		keycodes[i] = key;
		return i;
	}

	void updateKey(int code, bool press) {
		if(remap != -1) {
			controls[remap] = code;
			remap = -1;
			return;
		}

		sint i = 0;
		while(i < pressed.size() && code != controls[i])
			i++;

		if(i < pressed.size()) {
			pressed[i] = press;
			held[i] = press;
		}
	}

	void recieveEvent(sf::Event event, WindowSize *windowSize) {
		bool press = false;
		int code = 0;

		switch(event.type) {
			case sf::Event::KeyPressed: case sf::Event::KeyReleased:
				//Keyboard Input
				press = (event.type == sf::Event::KeyPressed);
				updateKey(event.key.code, press);
				break;
			case sf::Event::MouseButtonPressed: case sf::Event::MouseButtonReleased:
				//Mouse Buttons
				press = (event.type == sf::Event::MouseButtonPressed);
				code = Settings::MOUSEBUTTON[event.mouseButton.button];
				updateKey(code, press);
				break;
			case sf::Event::MouseWheelScrolled:
				//Mouse Wheel
				std::cout << event.mouseWheelScroll.delta << std::endl;
				updateKey(MOUSE_OFFSET+5, event.mouseWheelScroll.delta > 0);
				updateKey(MOUSE_OFFSET+6, event.mouseWheelScroll.delta < 0);
				break;
			case sf::Event::JoystickButtonPressed: case sf::Event::JoystickButtonReleased:
				//Joystick Buttons
				press = (event.type == sf::Event::JoystickButtonPressed);
				code = JOYSTICK_OFFSET + event.joystickButton.button;
				updateKey(code, press);
				break;
			case sf::Event::JoystickMoved:
				//Joysticks
				press = std::abs(event.joystickMove.position) > JOYSTICK_ZONE;
				code = JOYSTICK_OFFSET + 50 + (event.joystickMove.joystickId * 4) +
					Settings::JOYSTICKID[event.joystickMove.axis] * 2 + 
					(event.joystickMove.position > 0);
				updateKey(code, press);
				break;
			default:
				break;

		}
	}

	void clearPressed() {
		pressed.assign(pressed.size(), false);
	}

	void update(double time) {
		for(sint i = 0; i < held.size(); i++) {
			if(held[i]) {
				if(heldFunc != NULL)
					heldFunc(i % count);
				if(pressed[i] && pressedFunc != NULL)
					pressedFunc(i % count);
			}
		}

		clearPressed();
	}
};

class DirectionHandler : public InputHandler {
private:
	sf::Vector2f direction = sf::Vector2f(0, 0);
	int joystick = 0;
	int moving = 0;

public:
	bool joystickMovement = false;

	std::vector<std::string> listKeys(std::string field) {
		std::vector<std::string> keys = {
			field + "/up",
			field + "/down",
			field + "/left",
			field + "/right"
		};
		return keys;
	}

	DirectionHandler(std::vector<int> _controls, int layer, Node *parent = NULL)
	: InputHandler(_controls, layer, parent) {
		moving = addKey(-2);
	}

	DirectionHandler(std::vector<std::string> keys, int layer, Node *parent = NULL)
	: InputHandler(keys, layer, parent) {
		moving = addKey(-2);
	}

	DirectionHandler(std::string field, int layer, Node *parent = NULL)
	: DirectionHandler(listKeys(field), layer, parent) {
		joystick = Settings::getInt(field + "/joystick");
	}

	void update(double time) {
		direction = sf::Vector2f(0, 0);

		//Button Input
		for(sint i = 0; i < held.size(); i++) {
			if(held[i]) {
				joystickMovement = false;
				//Update direction
				switch(i % count) {
					case 0: // up
						direction.y--;
						break;
					case 1: // down
						direction.y++;
						break;
					case 2: // left
						direction.x--;
						break;
					case 3: // right
						direction.x++;
						break;
				}
			}
		}

		//Read from joystick
		int jid = (joystick - 1) / 4;
		if(direction == sf::Vector2f(0, 0) && joystick > 0) {
			int axes = ((joystick - 1) % 4) * 2;
			sf::Joystick::Axis xAxis = Settings::JOYSTICKAXIS[axes];
			sf::Joystick::Axis yAxis = Settings::JOYSTICKAXIS[axes + 1];
			if(sf::Joystick::hasAxis(jid, yAxis)) {
				float xPos = sf::Joystick::getAxisPosition(jid, xAxis);
				float yPos = sf::Joystick::getAxisPosition(jid, yAxis);

				//Update outside of dead zone
				if(std::abs(xPos) > JOYSTICK_ZONE || std::abs(yPos) > JOYSTICK_ZONE) {
					direction.x = xPos / 100;
					direction.y = yPos / 100;
					joystickMovement = true;
				} else if(joystickMovement) {
					direction.x = 0;
					direction.y = 0;
				}
			}
		}

		//Update moving placeholder
		updateKey(-2, direction != sf::Vector2f(0, 0));

		//Run lambda functions
		for(sint i = 0; i < held.size(); i++) {
			if(held[i]) {
				if(heldFunc != NULL)
					heldFunc(i % count);
				if(pressed[i] && pressedFunc != NULL)
					pressedFunc(i % count);
			}
		}

		clearPressed();
	}

	sf::Vector2f getDirection() {
		return direction;
	}

	sf::Vector2f getMovement(Node *node, double distance) {
		if(joystickMovement)
			return node->getPosition() + 
				sf::Vector2f(direction.x * distance, direction.y * distance);
		return node->getShiftedPosition(direction, distance);
	}
};
