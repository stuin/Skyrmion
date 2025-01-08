#include "../core/UpdateList.h"

std::vector<std::string> debugLayerKeys = {
	"/debug/layers/toggle0",
	"/debug/layers/toggle1",
	"/debug/layers/toggle2",
	"/debug/layers/toggle3",
	"/debug/layers/toggle4",
	"/debug/layers/toggle5",
	"/debug/layers/toggle6",
	"/debug/layers/toggle7",
	"/debug/layers/toggle8",
	"/debug/layers/toggle9"
};

class DebugLayers : public Node {
private:
	InputHandler input;

public:
	DebugLayers() : Node(UpdateList::getMaxLayer()+1, sf::Vector2i(16, 16), true),
	input(debugLayerKeys, getLayer(), this) {

		UpdateList::staticLayer(getLayer());

		input.pressedFunc = [](int i) {
			if(i < UpdateList::getMaxLayer())
				UpdateList::hideLayer(i, !UpdateList::isLayerHidden(i));
		};
	}
};

#if _DEBUG
	#define DEBUGLAYERS Settings::loadSettings("src/Skyrmion/res/debug_settings.json"); UpdateList::addNode(new DebugLayers())
#else
	#define DEBUGLAYERS void
#endif