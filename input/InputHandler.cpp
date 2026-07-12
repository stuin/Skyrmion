#include "InputHandler.h"

#include "../core/UpdateList.h"
#include "Settings.h"
#include "MovementEnums.h"

//Set controls directly
InputHandler::InputHandler(std::vector<int> _controls, int layer)
: UNode(layer) {

	for(int key : _controls)
		controls.push_back(Keybind(key));
	count = _controls.size();

	addListeners();
}

//Set controls through configurable settings
InputHandler::InputHandler(std::vector<std::string> keys, int layer)
: UNode(layer) {
	//Base keys
	for(sint i = 0; i < keys.size(); i++) {
		std::string keyname = Settings::getString(keys[i]);
		sint splitPlus = keyname.find('+');
		sint splitMinus = keyname.find('-');
		sint splitJoystick = keyname.find('%');
		if(splitPlus != std::string::npos) {
			//Split key combination
			std::string key1 = keyname.substr(0, splitPlus);
			std::string key2 = keyname.substr(splitPlus+1);
			controls.push_back(Keybind(Settings::mapKeycode(key1), keys[i], Settings::mapKeycode(key2)));
			UpdateList::watchKeycode(controls[i].key);
			UpdateList::watchKeycode(controls[i].comboKey);
			Settings::markKeycode.insert(key1);
			Settings::markKeycode.insert(key2);
		} else if(splitMinus != std::string::npos) {
			std::string key1 = keyname.substr(0, splitMinus);
			std::string key2 = keyname.substr(splitMinus+1);
			controls.push_back(Keybind(Settings::mapKeycode(key1), keys[i], -1, Settings::mapKeycode(key2)));
			UpdateList::watchKeycode(controls[i].key);
			UpdateList::watchKeycode(controls[i].avoidKey);
			Settings::markKeycode.insert(key1);
			Settings::markKeycode.insert(key2);
		} else if(splitJoystick != std::string::npos) {
			//Specific joystick/gamepad
			int player = std::stoi(keyname.substr(splitJoystick+1))*JOYSTICK_NEXT;
			controls.push_back(Keybind(Settings::getControl(keyname.substr(0, splitJoystick))+player, keys[i]));
		} else {
			//Single key
			controls.push_back(Keybind(Settings::getControl(keys[i]), keys[i]));
			UpdateList::watchKeycode(controls[i].key);
		}
	}
	count = controls.size();

	//Alternate keys
	for(int i = 1; i <= MAXALTS; i++) {
		for(sint j = 0; j < keys.size(); j++) {
			std::string s = controls[j].configName + "&" + std::to_string(i);
			controls.push_back(Keybind(Settings::getControl(s), s));
		}
	}

	addListeners();
}

//Subscribe to all input types
void InputHandler::addListeners() {
	UpdateList::addUNode(this);
	UpdateList::addListener(this, EVENT_KEYPRESS);
	UpdateList::addListener(this, EVENT_FOCUS);
	UpdateList::addListener(this, EVENT_SETTINGS);
}

//Register key through configurable settings
int InputHandler::addKey(std::string key) {
	int i = addKey(Settings::getControl(key));
	controls[i].configName = key;
	return i;
}

//Register key with pressed/held states
int InputHandler::addKey(int code, int alt) {
	controls.insert(controls.begin() + count, Keybind(code));
	UpdateList::watchKeycode(code);
	UpdateList::watchKeycode(alt);

	//Alternate keys
	for(int i = 2; i <= MAXALTS; i++)
		controls.insert(controls.begin() + count * i, Keybind(alt));

	return count++;
}

//Refresh inputs from settings
void InputHandler::refreshSettings() {
	for(sint i = 0; i < controls.size(); i++) {
		std::string name = controls[i].configName;
		if(name != "" && name[0] == '/' && controls[i].key != Settings::getControl(name)) {
			int code = Settings::getControl(name);
			//std::cout << name << code << "\n";
			if(code > 0) {
				controls[i].key = code;
				UpdateList::watchKeycode(controls[i].key);
				clearPressed(true);
			}
		}
	}
}

//Find if key is used and update pressed/held states
void InputHandler::updateKey(int code, bool press) {
	if(remap != -1 && remap < (long int)controls.size()) {
		controls[remap].key = code;
		remap = -1;
		return;
	}

	//std::cout << Settings::reverseKeycode(code) << " " << press << "\n";

	//Find key in controls
	for(sint i = 0; i < controls.size(); i++) {
		if(controls[i].key == code) {
			if(controls[i].comboKey == -1 && controls[i].avoidKey == -1) {
				if(controls[i].held != press) {
					controls[i].pressed = press;
					controls[i].pressed2 = press;
					controls[i].held = press;
				}
			} else if(controls[i].comboHeld[0] != press) {
				controls[i].comboHeld[0] = press;
				updateCombo(i);
			}
		} else if(controls[i].comboKey == code && controls[i].comboHeld[1] != press) {
			controls[i].comboHeld[1] = press;
			updateCombo(i);
		} else if(controls[i].avoidKey == code && controls[i].comboHeld[2] != press) {
			controls[i].comboHeld[2] = press;
			updateCombo(i);
		}
	}
}

void InputHandler::updateCombo(sint i) {
	bool comboPress = controls[i].comboHeld[0] && (controls[i].comboHeld[1] || controls[i].comboKey == -1)
	 && !controls[i].comboHeld[2];
	if(controls[i].held != comboPress) {
		controls[i].pressed = comboPress;
		controls[i].pressed2 = comboPress;
		controls[i].held = comboPress;
	}
}

//Clear pressed to separate newly pressed keys from held keys
void InputHandler::clearPressed(bool clearHeld) {
	for(sint i = 0; i < controls.size(); i++) {
		controls[i].pressed = false;

		//Mouse wheel special case
		if(controls[i].key == MOUSE_OFFSET+7 || controls[i].key == MOUSE_OFFSET+8 || clearHeld) {
			controls[i].held = false;
			controls[i].comboHeld[0] = false;
			controls[i].pressed2 = false;
		}
		if(controls[i].comboKey == MOUSE_OFFSET+7 || controls[i].comboKey == MOUSE_OFFSET+8 || clearHeld)
			controls[i].comboHeld[1] = false;
		if(controls[i].avoidKey == MOUSE_OFFSET+7 || controls[i].avoidKey == MOUSE_OFFSET+8 || clearHeld)
			controls[i].comboHeld[2] = false;
	}
}

bool InputHandler::isPressed(int kIndex) {
	if(controls[kIndex].pressed2) {
		controls[kIndex].pressed2 = false;
		return true;
	}
	return false;
}

bool InputHandler::isHeld(int kIndex) {
	return controls[kIndex].held;
}

//Convert input events to key updates
void InputHandler::recieveEvent(Event event) {
	if(event.type == EVENT_KEYPRESS)
		updateKey(event.code, event.down);
	else if(event.type == EVENT_FOCUS && event.down)
		clearPressed(true);
	else if(event.type == EVENT_SETTINGS)
		refreshSettings();
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

	clearPressed(false);
}

void InputHandler::printKey(sint i) {
	std::string keyName = Settings::reverseKeycode(controls[i].key);
	std::string comboName = Settings::reverseKeycode(controls[i].comboKey);
	std::string avoidName = Settings::reverseKeycode(controls[i].avoidKey);
	std::cout << controls[i].configName << " = " << controls[i].key << "/" << keyName << " ";
	std::cout << controls[i].comboKey << "/" << comboName << " -";
	std::cout << controls[i].avoidKey << "/" << avoidName << "\n";
}

void InputHandler::printKeys() {
	for(sint i = 0; i < controls.size(); i++)
		printKey(i);
}

DirectionHandler::DirectionHandler(std::vector<int> _controls, int layer)
: InputHandler(_controls, layer) {
	moving = addKey(-2);
	UpdateList::addListener(this, EVENT_JOYSTICK);
}

DirectionHandler::DirectionHandler(std::vector<std::string> keys, int layer)
: InputHandler(keys, layer) {
	moving = addKey(-2);
	UpdateList::addListener(this, EVENT_JOYSTICK);
}

DirectionHandler::DirectionHandler(std::string _field, int layer)
: DirectionHandler(listKeys(_field), layer) {
	field = _field;
	std::string s = field + "/joystick";
	joystick = Settings::getInt(s);
}

//Convert input events to key updates
void DirectionHandler::recieveEvent(Event event) {
	if(event.type == EVENT_KEYPRESS)
		updateKey(event.code, event.down);
	else if(event.type == EVENT_FOCUS && event.down)
		clearPressed(true);
	else if(event.type == EVENT_JOYSTICK && (event.code%4 == joystick-1 || event.code == joystick-1)) {
		if(event.down) {
			joystickMovement = true;
			joystickSim = false;
			joystickLast = event.code;
			joystickDirection = event.vector();
		} else if(!joystickSim && joystickLast == event.code)
			joystickDirection = event.vector();
	} else if(event.type == EVENT_JOYSTICK_SIM && event.code == joystick-1) {
		if(event.down) {
			joystickMovement = true;
			joystickSim = true;
			joystickDirection = event.vector();
		} else if(joystickSim)
			joystickDirection = event.vector();
	} else if(event.type == EVENT_SETTINGS) {
		refreshSettings();
		std::string s = field + "/joystick";
		joystick = Settings::getInt(s);
	}
}

//Calculate direction from joystick and keyboard
void DirectionHandler::update(double time) {
	direction = Vector2f(0, 0);

	//Button Input
	for(sint i = 0; i < controls.size(); i++) {
		if(controls[i].held) {
			joystickMovement = false;
			//Update direction
			switch(i % count) {
				case UP: // up
					direction.y--;
					break;
				case DOWN: // down
					direction.y++;
					break;
				case LEFT: // left
					direction.x--;
					break;
				case RIGHT: // right
					direction.x++;
					break;
			}
		}
	}

	//Read from joystick
	if(joystickDirection != Vector2f(0,0))
		direction += joystickDirection;
	if(distance(direction) > 1)
		direction = vectorLength(direction, 1);

	//Update moving placeholder key
	bool moved = direction != Vector2f(0, 0);
	if(moved != controls[moving].held)
		updateKey(-2, moved);

	//Run lambda functions
	for(sint i = 0; i < controls.size(); i++) {
		if(controls[i].held && i != (sint)moving) {
			if(heldFunc != NULL)
				heldFunc(i % count);
			if(controls[i].pressed && pressedFunc != NULL)
				pressedFunc(i % count);
		}
	}

	clearPressed(false);
}

//Get direct 2d direction based on inputs
Vector2f DirectionHandler::getDirection() {
	return direction;
}

//Limit keyboard distance to circle to match joystick
Vector2f DirectionHandler::getMovement(double distance) {
	if(joystickMovement)
		return Vector2f(direction.x * distance, direction.y * distance);
	return vectorLength(direction, distance);
}

//List field names for json settings
std::vector<std::string> DirectionHandler::listKeys(std::string field) {
	std::vector<std::string> keys = {
		field + "/up",
		field + "/right",
		field + "/down",
		field + "/left"
	};
	return keys;
}
