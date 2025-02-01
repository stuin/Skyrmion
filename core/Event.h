#pragma once

#include <map>

#define MOUSE_OFFSET 350
#define JOYSTICK_OFFSET 360
#define JOYSTICK_DEADZONE 0.05f

enum EVENT_TYPES {
	EVENT_KEYPRESS,
	EVENT_MOUSE,
	EVENT_SCROLL,
	EVENT_TOUCH,
	EVENT_JOYSTICK,
	EVENT_RESIZE,
	EVENT_FOCUS,
	EVENT_SUSPEND,
	EVENT_IMGUI,
	EVENT_CUSTOM,
	EVENT_MAX
};

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
};

bool operator==(const Event &first, const Event &second);
bool operator!=(const Event &first, const Event &second);