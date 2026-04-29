#include "../core/UpdateList.h"
#include "../input/InputHandler.h"
#include "../input/Settings.h"

#include "DebugLayers.hpp"
#include "DebugBreakpoints.hpp"

#include "ImguiFPS.hpp"
#include "ImguiNodes.hpp"
#include "ImguiEvents.hpp"
#include "ImguiResources.hpp"
#include "ImguiSettings.hpp"
#include "ImguiNoiseGen.hpp"

void addDebugTextures() {
	textureFiles().push_back("#DEBUG");
	textureFiles().push_back("res/debug/heatmapG.png");
	textureFiles().push_back("res/debug/heatmap.png");
	textureFiles().push_back("res/debug/colors.png");
}

void setupDebugTools() {
	Settings::loadSettings("res/debug/debug_settings.json", false);

	int debugLayer = layerNames().size();
	LayerData &data = UpdateList::getLayerData(debugLayer);
	data.name = "DEBUG";
	data.global = true;

	int debugTextures = std::distance(textureFiles().begin(),
		std::find(textureFiles().begin(), textureFiles().end(), "#DEBUG"));

	UpdateList::addUNode(new DebugLayers(-debugLayer));

	debugBreakpoints = new DebugBreakpoints(-debugLayer);
	UpdateList::addUNode(debugBreakpoints);

	new ImguiFPS(debugLayer);
	new ImguiNodes(debugTextures+3, debugLayer);
	new ImguiEvents(debugLayer);
	new ImguiResources(debugTextures+3, debugLayer);
	new ImguiSettings(debugLayer);
	new ImguiNoiseGen(debugTextures+1, debugLayer);
}