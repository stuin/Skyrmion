#pragma once

#include "Vector.h"

#define MOUSE_OFFSET 350
#define JOYSTICK_OFFSET 360
#define JOYSTICK_NEXT 50
#define JOYSTICK_DEADZONE 0.05f

enum EVENT_TYPES {
	EVENT_KEYPRESS,
	EVENT_MOUSE,
	EVENT_SCROLL,
	EVENT_TOUCH,
	EVENT_JOYSTICK,
	EVENT_JOYSTICK_SIM,
	EVENT_RESIZE,
	EVENT_FOCUS,
	EVENT_SUSPEND,
	EVENT_IMGUI,
	EVENT_NETWORK_CONNECT_SERVER,
	EVENT_NETWORK_CONNECT_CLIENT,
	EVENT_NETWORK_POSITION,
	EVENT_NETWORK,
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
	Event(int _type, bool _down, int _code, Vector2f pos) : Event(_type, _down, _code, pos.x, pos.y) {}

	Vector2f vector() {
		return Vector2f(x, y);
	}
};

bool operator==(const Event &first, const Event &second);
bool operator!=(const Event &first, const Event &second);
std::ostream& operator<<(std::ostream& os, const Event &event);

struct FrameTimer {
	int maxFrames = 0;
    int frame = 0;
    double maxTime = 0;
    double time = 0;

    FrameTimer(int _maxFrames, double _maxTime) {
    	maxFrames = _maxFrames;
    	maxTime = _maxTime;
    }

    int next(double delta) {
        if((time -= delta) <= 0) {
            time = maxTime;
            frame++;

            //Reset to start frame
            if(frame == maxFrames)
                frame = 0;
        }
        return frame;
    }
};