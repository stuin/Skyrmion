#include "../core/UpdateList.h"

class TouchscreenInput : public Node {
private:
	Vector2f dir;

public:
	TouchscreenInput(uint texture, Layer layer) : Node(layer, Vector2i(32,32)) {
		setTexture(texture);
		setTextureRect({16,16,32,32, 0,0,32,32,  0}, 0);
		setTextureRect({16,16,32,32, 32,0,32,32, 0}, 1);

		UpdateList::addNode(this);
		UpdateList::addListener(this, EVENT_TOUCH);
		UpdateList::addListener(this, EVENT_MOUSE);
	}

	void recieveEvent(Event event) {
		Vector2f pos = screenToGlobal(event.x, event.y) - getGPosition();
		if(event.down && distance(pos) < 32) {
			dir = vectorLength(pos, std::min(16.0f, distance(pos)));
		} else {
			dir = Vector2f(0,0);
		}
	}

	void update(double time) {
		setSPosition(UpdateList::getScreenRect().getSize() - Vector2f(128,128));
		setTextureRect({dir.x+16,dir.y+16,32,32, 32,0,32,32, 0}, 1);
	}

	Vector2f getDirection() {
		return dir;
	}
};