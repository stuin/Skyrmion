#include <functional>

#include "../core/Node.h"

//Minimal UNode that only runs an update function
class FuncNode : public UNode {
public:
	std::function<void(double)> func;

	FuncNode(int layer, std::function<void(double)> _func) : UNode(layer), func(_func) {
		UpdateList::addUNode(this);
	}

	void update(double time) {
		func(time);
	}
};

//Minimal UNode that only receives events
class EventNode : public UNode {
public:
	std::function<void(Event)> func;

	EventNode(int layer, int event, std::function<void(Event)> _func) : UNode(layer), func(_func) {
		UpdateList::addUNode(this);
		UpdateList::addListener(this, event);
	}

	void receiveEvent(Event event) {
		func(event);
	}
};