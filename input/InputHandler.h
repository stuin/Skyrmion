#pragma once

#include <algorithm>
#include <functional>

#include "../core/Node.h"

#define MAXALTS 2

/*
 * Read input events and collect them into remmappable keybinds
 */

struct Keybind {
	int key;
	std::string configName = "";
	long int combo = -1;
	long int duplicate = -1;
	bool pressed = false;
	bool held = false;
	bool query = true;

	Keybind(int _key) {
		key = _key;
	}
	Keybind(int _key, const std::string& _configName, long int _combo=-1) {
		key = _key;
		configName = _configName;
		combo = _combo;
		duplicate = -1;
		if(combo >= 0)
			query = false;
	}
};

class InputHandler : public Node {
private:
	void addListeners();

public:
	std::vector<Keybind> controls;
	std::function<void(int)> pressedFunc = NULL;
	std::function<void(int)> heldFunc = NULL;

	long int remap = -1;
	sint count = 0;

	InputHandler(std::vector<int> _controls, int layer, Node *parent = NULL);
	InputHandler(std::vector<std::string> keys, int layer, Node *parent = NULL);

	//Key management
	int addKey(int code, int alt=0);
	int addKey(std::string key);

	//Key press and unpress
	void updateKey(int code, bool press);
	void clearPressed(bool clearHeld=false);

	//System updates loop
	void recieveEvent(Event event);
	void update(double time);
};

//Input with built in joystick and directional keyboard support
class DirectionHandler : public InputHandler {
private:
	Vector2f direction = Vector2f(0, 0);
	Vector2f joystickDirection = Vector2f(0,0);

	int joystick = 0;
	int joystickLast = 0;
	bool joystickSim = false;

public:
	bool joystickMovement = false;
	int moving = 0;

	DirectionHandler(std::vector<int> _controls, int layer, Node *parent = NULL);
	DirectionHandler(std::vector<std::string> keys, int layer, Node *parent = NULL);
	DirectionHandler(std::string field, int layer, Node *parent = NULL);

	Vector2f getDirection();
	Vector2f getMovement(double distance);

	//System updates loop
	void recieveEvent(Event event);
	void update(double time);

	static std::vector<std::string> listKeys(std::string field);
};


