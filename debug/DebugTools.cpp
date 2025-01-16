#include "../core/UpdateList.h"
#include "../input/InputHandler.h"

#include "DebugLayers.hpp"
#include "DebugBreakpoints.hpp"

void addDebugTextures() {
	textureFiles().push_back("#DEBUG");
	textureFiles().push_back("src/Skyrmion/res/heatmapG.png");
	textureFiles().push_back("src/Skyrmion/res/heatmap.png");
	textureFiles().push_back("src/Skyrmion/res/colors.png");
}

void setupDebugTools() {
	Settings::loadSettings("src/Skyrmion/res/debug_settings.json");

	int debugLayer = UpdateList::getLayerCount();
	UpdateList::staticLayer(debugLayer);
	UpdateList::screenSpaceLayer(debugLayer);
	layerNames().push_back("DEBUG");

	UpdateList::addNode(new DebugLayers(debugLayer));

	debugBreakpoints = new DebugBreakpoints(debugLayer);
	UpdateList::addNode(debugBreakpoints);
}