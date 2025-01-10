/*
 * Debug tool to show/hide layers in game with ctrl+number
 */

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
	DebugLayers(int layer) : Node(layer, sf::Vector2i(16, 16), true),
	input(debugLayerKeys, layer, this) {

		input.pressedFunc = [layer](int i) {
			if(i < layer)
				UpdateList::hideLayer(i, !UpdateList::isLayerHidden(i));
		};
	}
};