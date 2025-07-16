#include "InputHandler.h"

#include "Settings.h"
#include "../core/UpdateList.h"

nlohmann::json Settings::data({});
std::vector<std::pair<std::string, std::string>> Settings::edits;

//Set controls directly
InputHandler::InputHandler(std::vector<int> _controls, int layer, Node *parent)
: Node(layer, Vector2i(16, 16), true, parent) {

	for(int key : _controls)
		controls.push_back(Keybind(key));
	count = _controls.size();

	addListeners();
}

//Set controls through configurable settings
InputHandler::InputHandler(std::vector<std::string> keys, int layer, Node *parent)
: Node(layer, Vector2i(16, 16), true, parent) {
	//Base keys
	sint startSize = keys.size();
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
				splitI = keyname.find('%');
				if(splitI != std::string::npos) {
					//Specific joystick/gamepad
					int player = std::stoi(keyname.substr(splitI+1))*JOYSTICK_NEXT;
					controls.push_back(Keybind(Settings::getControl(keyname.substr(0, splitI))+player, keys[i]));
				} else {
					//Single key
					controls.push_back(Keybind(Settings::getControl(keys[i]), keys[i]));
				}
			}
		}
	}
	count = keys.size();

	//Alternate keys
	for(int i = 1; i <= MAXALTS; i++) {
		for(sint j = 0; j < keys.size(); j++) {
			std::string s = controls[j].configName + "&" + std::to_string(i);
			if(j >= startSize)
				controls.push_back(Keybind(0, s));
			else
				controls.push_back(Keybind(Settings::getControl(s), s));
		}
	}

	//Mark duplicates
	for(sint i = 0; i < controls.size(); i++) {
		if(controls[i].key > 0) {
			for(sint j = i + 1; j < controls.size() && controls[i].duplicate == -1; j++) {
				if(controls[i].key == controls[j].key) {
					controls[i].duplicate = j;
					controls[j].query = false;
				}
			}
			UpdateList::watchKeycode(controls[i].key);
		}
	}

	addListeners();
}

//Subscribe to all input types
void InputHandler::addListeners() {
	UpdateList::addNode(this);
	UpdateList::addListener(this, EVENT_KEYPRESS);
	UpdateList::addListener(this, EVENT_FOCUS);
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

	//Correct stored indexes
	for(sint i = 0; i < controls.size(); i++) {
		if(controls[i].combo > -1 && (sint)controls[i].combo > count)
			controls[i].combo += controls[i].combo / count;
		if(controls[i].duplicate > -1 && (sint)controls[i].duplicate > count)
			controls[i].duplicate += controls[i].duplicate / count;
	}

	return count++;
}

//Find if key is used and update pressed/held states
void InputHandler::updateKey(int code, bool press) {
	if(remap != -1 && remap < (long int)controls.size()) {
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
		while(d != -1 && d < (long int)controls.size()) {
			controls[d].pressed = press;
			controls[d].held = press;
			if(controls[d].duplicate != d)
				d = controls[d].duplicate;
			else
				d = -1;
		}
	}

	//Check for combo key
	if(i < controls.size() && controls[i].combo == -3) {
		sint j = 0;
		while(j < controls.size() && (long int)i/2 != controls[j].combo/2)
			j++;

		//Update press/held
		if(j < controls.size()) {
			int k = controls[j].combo;
			press = controls[k].held && controls[k+1].held;
			if(controls[j].held != press) {
				controls[j].pressed = press;
				controls[j].held = press;
			}
		}
	}
}

//Clear pressed to separate newly pressed keys from held keys
void InputHandler::clearPressed(bool clearHeld) {
	for(sint i = 0; i < controls.size(); i++) {
		controls[i].pressed = false;

		//Mouse wheel special case
		if(controls[i].key == MOUSE_OFFSET+7 || controls[i].key == MOUSE_OFFSET+8 || clearHeld)
			controls[i].held = false;
	}
}

//Convert input events to key updates
void InputHandler::recieveEvent(Event event) {
	if(event.type == EVENT_KEYPRESS)
		updateKey(event.code, event.down);
	else if(event.type == EVENT_FOCUS && event.down)
		clearPressed(true);
}

//Run key press functions
void InputHandler::update(double time) {
	for(sint i = 0; i < controls.size(); i++) {
		if(controls[i].held && controls[i].combo != -3) {
			if(heldFunc != NULL)
				heldFunc(i % count);
			if(controls[i].pressed && pressedFunc != NULL)
				pressedFunc(i % count);
		}
	}

	clearPressed(false);
}

DirectionHandler::DirectionHandler(std::vector<int> _controls, int layer, Node *parent)
: InputHandler(_controls, layer, parent) {
	moving = addKey(-2);
	UpdateList::addListener(this, EVENT_JOYSTICK);
}

DirectionHandler::DirectionHandler(std::vector<std::string> keys, int layer, Node *parent)
: InputHandler(keys, layer, parent) {
	moving = addKey(-2);
	UpdateList::addListener(this, EVENT_JOYSTICK);
}

DirectionHandler::DirectionHandler(std::string field, int layer, Node *parent)
: DirectionHandler(listKeys(field), layer, parent) {
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
				case 0: // up
					direction.y--;
					break;
				case 1: // right
					direction.x++;
					break;
				case 2: // down
					direction.y++;
					break;
				case 3: // left
					direction.x--;
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
		if(controls[i].held) {
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
