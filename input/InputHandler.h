#pragma once

#include <cstdlib>

#include "Settings.h"
#include "MovementEnums.h"
#include "../core/UpdateList.h"

#define MAXALTS 2
#define JOYSTICK_ZONE 5.0f

struct Keybind {
	int key;
	std::string configName = "";
	long int combo = -1;
	long int duplicate = -1;
	bool pressed = false;
	bool held = false;

	Keybind(int _key) {
		key = _key;
	}
	Keybind(int _key, const std::string& _configName, int _combo=-1) {
		key = _key;
		configName = _configName;
		combo = _combo;
		duplicate = -1;
	}
};

class InputHandler : public Node {
private:
	void add_listeners();

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
	void clearPressed();

	//System updates loop
	void recieveEvent(sf::Event event, WindowSize *windowSize);
	void update(double time);
};

//Input with built in joystick and directional keyboard support
class DirectionHandler : public InputHandler {
private:
	sf::Vector2f direction = sf::Vector2f(0, 0);
	int joystick = 0;

public:
	bool joystickMovement = false;
	int moving = 0;

	DirectionHandler(std::vector<int> _controls, int layer, Node *parent = NULL);
	DirectionHandler(std::vector<std::string> keys, int layer, Node *parent = NULL);
	DirectionHandler(std::string field, int layer, Node *parent = NULL);

	sf::Vector2f getDirection();
	sf::Vector2f getMovement(double distance);

	void update(double time);

	static std::vector<std::string> listKeys(std::string field);
};


