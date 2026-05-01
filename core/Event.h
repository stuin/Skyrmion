#pragma once

#include "Vector.h"

//Control input constants
#define MOUSE_OFFSET 350
#define JOYSTICK_OFFSET 360
#define JOYSTICK_NEXT 50
#define JOYSTICK_DEADZONE 0.05f

//Macro to build enum + array of names
#define GENERATE_TYPES(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

#define NAMED_ENUM(ENUM) enum ENUM##_TYPES { ENUM##_FOREACH(GENERATE_TYPES) }; static std::vector<std::string> ENUM##_NAMES = { ENUM##_FOREACH(GENERATE_STRING) };

//List of event types
#define EVENT_FOREACH(E) \
	E(EVENT_KEYPRESS) \
	E(EVENT_MOUSE) \
	E(EVENT_SCROLL) \
	E(EVENT_TOUCH) \
	E(EVENT_JOYSTICK) \
	E(EVENT_JOYSTICK_SIM) \
	E(EVENT_RESIZE) \
	E(EVENT_FOCUS) \
	E(EVENT_SUSPEND) \
	E(EVENT_SETTINGS) \
	E(EVENT_IMGUI) \
	E(EVENT_NETWORK_CONNECT_SERVER) \
	E(EVENT_NETWORK_CONNECT_CLIENT) \
	E(EVENT_NETWORK_POSITION) \
	E(EVENT_NETWORK) \
	E(EVENT_CUSTOM) \
	E(EVENT_MAX) \

NAMED_ENUM(EVENT);

//Minimized event data
struct Event {
	int type;
	bool down;
	int code;
	float x = 0;
	float y = 0;

	Event() {
		type = EVENT_KEYPRESS;
		down = false;
		code = 0;
	}

	Event(int _type, bool _down, int _code) {
		type = _type;
		down = _down;
		code = _code;
	}
	Event(int _type, bool _down, int _code, float _x, float _y) {
		type = _type;
		down = _down;
		code = _code;
		x = _x;
		y = _y;
	}
	Event(int _type, bool _down, int _code, Vector2f pos) : Event(_type, _down, _code, pos.x, pos.y) {}

	Vector2f vector() {
		return Vector2f(x, y);
	}
};

//Event operators (Vector.cpp)
bool operator==(const Event &first, const Event &second);
bool operator!=(const Event &first, const Event &second);
std::ostream& operator<<(std::ostream& os, const Event &event);

class IO {
public:
	//Engine compatible file read/write
	static char *openFile(std::string filename);
	static void closeFile(char *file);
	static void writeFile(std::string filename, char *text);
	static void writeFile(std::string filename, std::string text);
};