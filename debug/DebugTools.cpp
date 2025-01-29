#include "../core/UpdateList.h"
#include "../input/InputHandler.h"

#include "DebugLayers.hpp"
#include "DebugBreakpoints.hpp"

void addDebugTextures() {
	textureFiles().push_back("#DEBUG");
	textureFiles().push_back("res/debug/heatmapG.png");
	textureFiles().push_back("res/debug/heatmap.png");
	textureFiles().push_back("res/debug/colors.png");
}

void setupDebugTools() {
	Settings::loadSettings("res/debug/debug_settings.json");

	int debugLayer = UpdateList::getLayerCount();
	LayerData &data = UpdateList::getLayerData(debugLayer);
	data.name = "DEBUG";
	data.global = true;

	UpdateList::addNode(new DebugLayers(debugLayer));

	debugBreakpoints = new DebugBreakpoints(debugLayer);
	UpdateList::addNode(debugBreakpoints);
}