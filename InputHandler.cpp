#include "InputHandler.h"

void InputHandler::add_listeners() {
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

InputHandler::InputHandler(std::vector<int> _controls, int layer, Node *parent) 
: Node(layer, sf::Vector2i(16, 16), true, parent), controls(_controls) {
	keycodes.assign(_controls.size(), "");
	pressed.assign(_controls.size(), false);
	held.assign(_controls.size(), false);
	count = _controls.size();

	add_listeners();
}

InputHandler::InputHandler(std::vector<std::string> keys, int layer, Node *parent)
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

int InputHandler::addKey(int code) {
	controls.insert(controls.begin() + count, code);
	keycodes.insert(keycodes.begin() + count, "");
	pressed.insert(pressed.begin() + count, false);
	held.insert(held.begin() + count, false);
	return count++;
}

int InputHandler::addKey(std::string key) {
	int i = addKey(Settings::getControl(key));
	keycodes[i] = key;
	return i;
}

void InputHandler::updateKey(int code, bool press) {
	if(remap != -1) {
		controls[remap] = code;
		remap = -1;
		return;
	}

	sint i = 0;
	while(i < pressed.size() && code != controls[i])
		i++;

	if(i < pressed.size() && held[i] != press) {
		pressed[i] = press;
		held[i] = press;
	}
}

void InputHandler::clearPressed() {
	pressed.assign(pressed.size(), false);
}

void InputHandler::recieveEvent(sf::Event event, WindowSize *windowSize) {
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

void InputHandler::update(double time) {
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


DirectionHandler::DirectionHandler(std::vector<int> _controls, int layer, Node *parent)
: InputHandler(_controls, layer, parent) {
	moving = addKey(-2);
}

DirectionHandler::DirectionHandler(std::vector<std::string> keys, int layer, Node *parent)
: InputHandler(keys, layer, parent) {
	moving = addKey(-2);
}

DirectionHandler::DirectionHandler(std::string field, int layer, Node *parent)
: DirectionHandler(listKeys(field), layer, parent) {
	joystick = Settings::getInt(field + "/joystick");
}

void DirectionHandler::update(double time) {
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
				case 1: // left
					direction.x--;
					break;
				case 2: // down
					direction.y++;
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
	bool moved = direction != sf::Vector2f(0, 0);
	if(moved != held[moving])
		updateKey(-2, moved);

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

sf::Vector2f DirectionHandler::getDirection() {
	return direction;
}

sf::Vector2f DirectionHandler::getMovement(double distance) {
	if(joystickMovement)
		return sf::Vector2f(direction.x * distance, direction.y * distance);
	return Node::vectorLength(direction, distance);
}

//List field names for json settings
std::vector<std::string> DirectionHandler::listKeys(std::string field) {
	std::vector<std::string> keys = {
		field + "/up",
		field + "/left",
		field + "/down",
		field + "/right"
	};
	return keys;
}
