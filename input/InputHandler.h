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
	int comboKey = -1;
	int avoidKey = -1;
	bool pressed = false;
	bool pressed2 = false;
	bool held = false;
	bool comboHeld[3] = {false};

	Keybind(int _key) {
		key = _key;
	}
	Keybind(int _key, const std::string& _configName, int _combo=-1, int _avoid=-1) {
		key = _key;
		configName = _configName;
		comboKey = _combo;
		avoidKey = _avoid;
	}
};

class InputHandler : public UNode {
private:
	void addListeners();

public:
	std::vector<Keybind> controls;
	std::function<void(int)> pressedFunc = NULL;
	std::function<void(int)> heldFunc = NULL;

	long int remap = -1;
	sint count = 0;

	InputHandler(std::vector<int> _controls, int layer);
	InputHandler(std::vector<std::string> keys, int layer);

	//Key management
	int addKey(int code, int alt=0);
	int addKey(std::string key);
	void refreshSettings();

	//Key press and unpress
	void updateKey(int code, bool press);
	void updateCombo(sint i);
	void clearPressed(bool clearHeld=false);

	//Simple key querying
	bool isPressed(int kIndex);
	bool isHeld(int kIndex);

	//System updates loop
	void recieveEvent(Event event);
	void update(double time);

	void printKey(sint i);
	void printKeys();
};

//Input with built in joystick and directional keyboard support
class DirectionHandler : public InputHandler {
private:
	Vector2f direction = Vector2f(0, 0);
	Vector2f joystickDirection = Vector2f(0,0);
	std::string field;

	int joystick = 0;
	int joystickLast = 0;
	bool joystickSim = false;

public:
	bool joystickMovement = false;
	int moving = 0;

	DirectionHandler(std::vector<int> _controls, int layer);
	DirectionHandler(std::vector<std::string> keys, int layer);
	DirectionHandler(std::string field, int layer);

	Vector2f getDirection();
	Vector2f getMovement(double distance);

	//System updates loop
	void recieveEvent(Event event);
	void update(double time);

	static std::vector<std::string> listKeys(std::string field);
};


