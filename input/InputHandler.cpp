#include "InputHandler.h"

//Set controls directly
InputHandler::InputHandler(std::vector<int> _controls, int layer, Node *parent)
: Node(layer, sf::Vector2i(16, 16), true, parent) {

	for(int key : _controls)
		controls.push_back(Keybind(key));
	count = _controls.size();

	add_listeners();
}

//Set controls through configurable settings
InputHandler::InputHandler(std::vector<std::string> keys, int layer, Node *parent)
: Node(layer, sf::Vector2i(16, 16), true, parent) {

	//Base keys
	int startSize = keys.size();
	for(sint i = 0; i < keys.size(); i++) {
		if(i >= startSize) {
			//Part of key combination
			controls.push_back(Keybind(Settings::mapKeycode(keys[i]), keys[i], -3));
		} else {
			std::string keyname = Settings::getString(keys[i]);
			sint splitI = keyname.find('+');
			if(splitI != std::string::npos) {
				//Split key combination
				controls.push_back(Keybind(-3, keys[i], keys.size()));
				keys.push_back(keyname.substr(0, splitI));
				keys.push_back(keyname.substr(splitI+1));
			} else {
				//Single key
				controls.push_back(Keybind(Settings::getControl(keys[i]), keys[i]));
			}
		}
	}
	count = keys.size();

	//Alternate keys
	for(int i = 1; i < MAXALTS; i++) {
		for(sint j = 0; j < keys.size(); j++) {
			std::string s = controls[j].configName;
			if(j >= startSize)
				controls.push_back(Keybind(0, s + "&" + std::to_string(i)));
			else
				controls.push_back(Keybind(Settings::getControl(s + "&" + std::to_string(i)), s + "&" + std::to_string(i)));
		}
	}

	//Mark duplicates
	for(sint i = 0; i < controls.size(); i++) {
		if(controls[i].key > 0) {
			for(sint j = i + 1; j < controls.size() && controls[i].duplicate == -1; j++) {
				if(controls[i].key == controls[j].key)
					controls[i].duplicate = j;
			}
		}
	}

	add_listeners();
}

//Subscribe to all input types
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

//Register key through configurable settings
int InputHandler::addKey(std::string key) {
	int i = addKey(Settings::getControl(key));
	controls[i].configName = key;
	return i;
}

//Register key with pressed/held states
int InputHandler::addKey(int code) {
	controls.insert(controls.begin() + count, Keybind(code));

	//Correct stored indexes
	for(sint i = 0; i < controls.size(); i++) {
		if(controls[i].combo > -1 && controls[i].combo > count)
			controls[i].combo++;
		if(controls[i].duplicate > -1 && controls[i].duplicate > count)
			controls[i].duplicate++;
	}

	return count++;
}

//Find if key is used and update pressed/held states
void InputHandler::updateKey(int code, bool press) {
	if(remap != -1 && remap < controls.size()) {
		controls[remap].key = code;
		remap = -1;
		return;
	}

	//Find key in controls
	sint i = 0;
	while(i < controls.size() && code != controls[i].key)
		i++;

	//Update press/held
	if(i < controls.size() && controls[i].held != press) {
		controls[i].pressed = press;
		controls[i].held = press;

		long int d = controls[i].duplicate;
		while(d != -1 && d < controls.size()) {
			controls[d].pressed = press;
			controls[d].held = press;
			if(controls[d].duplicate != d)
				d = controls[d].duplicate;
			else
				d = -1;
		}
	}

	//Check for combo key
	if(controls[i].combo == -3) {
		sint j = 0;
		while(j < controls.size() && i/2 != controls[j].combo/2)
			j++;

		//Update press/held
		if(j < controls.size()) {
			i = controls[j].combo;
			press = controls[i].held && controls[i+1].held;
			if(controls[j].held != press) {
				controls[j].pressed = press;
				controls[j].held = press;
			}
		}
	}
}

//Clear pressed to separate newly pressed keys from held keys
void InputHandler::clearPressed() {
	for(Keybind key : controls)
		key.pressed = false;

	//Mouse wheel special case
	for(long unsigned int i = 0; i < controls.size(); i++)
		if(controls[i].key == MOUSE_OFFSET+5 || controls[i].key == MOUSE_OFFSET+6)
			controls[i].held = false;
}

//Convert draw thread input events to key updates
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

//Run key press functions
void InputHandler::update(double time) {
	for(sint i = 0; i < controls.size(); i++) {
		if(controls[i].held) {
			if(heldFunc != NULL)
				heldFunc(i % count);
			if(controls[i].pressed && pressedFunc != NULL)
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

//Calculate direction from joystick and keyboard
void DirectionHandler::update(double time) {
	direction = sf::Vector2f(0, 0);

	//Button Input
	for(sint i = 0; i < controls.size(); i++) {
		if(controls[i].held) {
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

	//Update moving placeholder key
	bool moved = direction != sf::Vector2f(0, 0);
	if(moved != controls[moving].held)
		updateKey(-2, moved);

	//Run lambda functions
	for(sint i = 0; i < controls.size(); i++) {
		if(controls[i].held) {
			if(heldFunc != NULL)
				heldFunc(i % count);
			if(controls[i].pressed && pressedFunc != NULL)
				pressedFunc(i % count);
		}
	}

	clearPressed();
}

//Get direct 2d direction based on inputs
sf::Vector2f DirectionHandler::getDirection() {
	return direction;
}

//Limit keyboard distance to circle to match joystick
sf::Vector2f DirectionHandler::getMovement(double distance) {
	if(joystickMovement)
		return sf::Vector2f(direction.x * distance, direction.y * distance);
	return vectorLength(direction, distance);
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
