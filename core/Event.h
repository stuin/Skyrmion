#pragma once

#include "Vector.h"
#include "Enum.h"

//Control input constants
#define MOUSE_OFFSET 350
#define JOYSTICK_OFFSET 360
#define JOYSTICK_NEXT 50
#define JOYSTICK_DEADZONE 0.05f

//List of event types
//Overflow types are sorted into EVENT_CUSTOM
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
	E(EVENT_BUFFER) \
	E(EVENT_UNIFORM) \
	E(EVENT_IMGUI) \
	E(EVENT_AUDIO) \
	E(EVENT_NETWORK_CONNECT_SERVER) \
	E(EVENT_NETWORK_CONNECT_CLIENT) \
	E(EVENT_NETWORK_POSITION1) \
	E(EVENT_NETWORK_POSITION2) \
	E(EVENT_NETWORK_POSITION3) \
	E(EVENT_NETWORK_POSITION4) \
	E(EVENT_NETWORK_CUSTOM1) \
	E(EVENT_NETWORK_CUSTOM2) \
	E(EVENT_NETWORK_CUSTOM3) \
	E(EVENT_NETWORK_CUSTOM4) \
	E(EVENT_CUSTOM1) \
	E(EVENT_CUSTOM2) \
	E(EVENT_CUSTOM3) \
	E(EVENT_CUSTOM4) \
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
	static void deleteFile(std::string filename);
	static bool hasFile(std::string filename);
	static int fileSize(std::string filename);
	static void createFolder(std::string filename);

	//Add event to UpdateList queue
	static void queueEvent(Event event);
	static void queueEvent(int type, bool down, int code, float x=0, float y=0);
};