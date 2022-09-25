#include "UpdateList.h"
#include "Settings.h"

class InputHandler : public Node {
public:
	std::vector<int> controls;
	std::vector<bool> pressed;
	std::vector<bool> held;
	std::function<void(int)> pressedFunc = NULL;
	std::function<void(int)> heldFunc = NULL;

	int remap = -1;

	InputHandler(std::vector<int> _controls, int layer, Node *parent = NULL) 
	: Node(layer, sf::Vector2i(16, 16), true, parent), controls(_controls) {
		pressed.assign(_controls.size(), false);
		held.assign(_controls.size(), false);

		UpdateList::addNode(this);
		UpdateList::addListener(this, sf::Event::KeyPressed);
		UpdateList::addListener(this, sf::Event::KeyReleased);
		UpdateList::addListener(this, sf::Event::MouseButtonPressed);
		UpdateList::addListener(this, sf::Event::MouseButtonReleased);
		UpdateList::addListener(this, sf::Event::MouseWheelScrolled);
		UpdateList::addListener(this, sf::Event::JoystickButtonPressed);
		UpdateList::addListener(this, sf::Event::JoystickButtonReleased);
	}

	InputHandler(std::vector<std::string> keys, int layer, Node *parent = NULL)
	: InputHandler(Settings::getControls(keys), layer, parent) {

	}

	int addKey(int code) {
		controls.push_back(code);
		pressed.push_back(false);
		held.push_back(false);
		return controls.size() - 1;
	}

	int addKey(std::string key) {
		return addKey(Settings::getControl(key));
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

		if(code == controls[i]) {
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
	int joystick = 0;

public:
	bool joystickMovement = false;

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

	DirectionHandler(std::vector<int> _controls, int count, int layer, Node *parent = NULL)
	: InputHandler(_controls, count, layer, parent) {
	
	}

	DirectionHandler(std::vector<std::string> keys, int layer, Node *parent = NULL)
	: InputHandler(keys, layer, parent) {

	}

	DirectionHandler(std::string field, int layer, Node *parent = NULL)
	: DirectionHandler(listKeys(field), layer, parent) {
		joystick = Settings::getInt(field + "/joystick");
	}

	void update(double time) {
		sf::Vector2f _direction = sf::Vector2f(0, 0);
		int last = 0;

		//Button Input
		for(sint i = 0; i < held.size(); i++) {
			if(held[i]) {
				last = i + 1;
				joystickMovement = false;
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
			}
		}

		//Read from joystick
		if(joystick > 0) {
			int axes = ((joystick - 1) % 4) * 2;
			sf::Joystick::Axis xAxis = Settings::AXISID[axes];
			sf::Joystick::Axis yAxis = Settings::AXISID[axes + 1];
			float xPos = sf::Joystick::getAxisPosition(joystick / 4, xAxis);
			float yPos = sf::Joystick::getAxisPosition(joystick / 4, yAxis);

			//Update outside of dead zone
			if(xPos > JOYSTICK_ZONE || xPos < -JOYSTICK_ZONE ||
					yPos > JOYSTICK_ZONE || yPos < -JOYSTICK_ZONE) {
				//std::cout << xPos << " " << yPos << std::endl;
				_direction.x = xPos / 100;
				_direction.y = yPos / 100;
				last = -1;
				joystickMovement = true;
			} else if(joystickMovement) {
				_direction.x = 0;
				_direction.y = 0;
			}
		}

		//Run lambda functions
		if(last != 0) {
			if(heldFunc != NULL)
				heldFunc(last);
			if(pressed[last] && pressedFunc != NULL)
				pressedFunc(last);
		}

		direction = _direction;
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
