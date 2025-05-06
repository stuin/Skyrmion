#include "../core/UpdateList.h"
#include "../input/InputHandler.h"
#include "../input/Settings.h"

#include "DebugLayers.hpp"
#include "DebugBreakpoints.hpp"

#include "ImguiFPS.hpp"
#include "ImguiNodes.hpp"
#include "ImguiEvents.hpp"
#include "ImguiColorPicker.hpp"
#include "ImguiNoiseGen.hpp"

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

	int debugTextures = std::distance(textureFiles().begin(),
		std::find(textureFiles().begin(), textureFiles().end(), "#DEBUG"));

	UpdateList::addNode(new DebugLayers(debugLayer));

	debugBreakpoints = new DebugBreakpoints(debugLayer);
	UpdateList::addNode(debugBreakpoints);

	new ImguiFPS(debugLayer);
	new ImguiNodes(debugTextures+3, debugLayer);
	new ImguiEvents(debugLayer);
	new ImguiColorPicker(debugTextures+3, debugLayer);
	new ImguiNoiseGen(debugTextures+1, debugLayer);
}