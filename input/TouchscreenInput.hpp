#include "../core/UpdateList.h"

class TouchscreenJoystick : public Node {
private:
	Vector2f dir;
	int joystick;
	bool touch = false;

public:
	TouchscreenJoystick(uint texture, int _joystick, Layer layer) : Node(layer, Vector2i(32,32)) {
		setTexture(texture);
		setTextureRect({16,16,32,32, 0,0,32,32,  0}, 0);
		setTextureRect({16,16,32,32, 32,0,32,32, 0}, 1);

		joystick = _joystick;

		#ifndef PLATFORM_ANDROID
		UpdateList::hideLayer(layer);
		#endif

		UpdateList::addNode(this);
		UpdateList::addListener(this, EVENT_TOUCH);
		UpdateList::addListener(this, EVENT_MOUSE);
	}

	void recieveEvent(Event event) {
		if(event.type == EVENT_TOUCH && event.down)
			UpdateList::hideLayer(getLayer(), false);
		if(isHidden())
			return;

		Vector2f pos = screenToGlobal(event.x, event.y) - getGPosition();
		float dis = distance(pos);
		if(event.down && event.code == 0 && dis < 32.0f && dis > 2.0f) {
			dir = vectorLength(pos, std::min(16.0f, dis));
			setTextureRect({dir.x+16,dir.y+16,32,32, 32,0,32,32, 0}, 1);
			UpdateList::queueEvent({EVENT_JOYSTICK_SIM, true, joystick-1, dir.x/16.0f, dir.y/16.0f});
			touch = event.type == EVENT_TOUCH;
		} else if(event.type == (touch ? EVENT_TOUCH : EVENT_MOUSE)) {
			dir = Vector2f(0,0);
			setTextureRect({dir.x+16,dir.y+16,32,32, 32,0,32,32, 0}, 1);
			UpdateList::queueEvent({EVENT_JOYSTICK_SIM, false, joystick-1, 0, 0});
		}
	}

	void update(double time) {
		setSPosition(Vector2f(-128,-128));
	}

	Vector2f getDirection() {
		return dir;
	}
};