#include "Settings.h"
#include "UpdateList.h"

#define MAXALTS 2
#define JOYSTICK_ZONE 5.0f

class InputHandler : public Node {
private:
	void add_listeners();

public:
	std::vector<int> controls;
	std::vector<std::string> keycodes;
	std::vector<bool> pressed;
	std::vector<bool> held;
	std::function<void(int)> pressedFunc = NULL;
	std::function<void(int)> heldFunc = NULL;

	int remap = -1;
	int count = 0;

	InputHandler(std::vector<int> _controls, int layer, Node *parent = NULL);
	InputHandler(std::vector<std::string> keys, int layer, Node *parent = NULL);

	//Key management
	int addKey(int code);
	int addKey(std::string key);

	//Key press and unpress
	void updateKey(int code, bool press);
	void clearPressed();

	//System updates loop
	void recieveEvent(sf::Event event, WindowSize *windowSize);
	void update(double time);
};

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
	sf::Vector2f getMovement(Node *node, double distance);

	void update(double time);

	static std::vector<std::string> listKeys(std::string field);
};


