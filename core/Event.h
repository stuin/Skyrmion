#pragma once

#include <map>

#define MOUSE_OFFSET 350
#define JOYSTICK_OFFSET 360

enum EVENT_TYPES {
	EVENT_KEYPRESS,
	EVENT_MOUSE,
	EVENT_SCROLL,
	EVENT_TOUCH,
	EVENT_RESIZE,
	EVENT_FOCUS,
	EVENT_SUSPEND,
	EVENT_IMGUI,
	EVENT_MAX
};

struct Event {
	int type;
	bool down;
	int code;
	float x;
	float y;

	Event(int _type, bool _down, int _code) {
		type = _type;
		down = _down;
		code = _code;
		x = 0;
		y = 0;
	}
	Event(int _type, bool _down, int _code, float _x, float _y) {
		type = _type;
		down = _down;
		code = _code;
		x = _x;
		y = _y;
	}
};