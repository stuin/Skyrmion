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
	WindowConfig config = windowConfig();
	config.textureFiles.push_back("_DEBUG");
	config.textureFiles.push_back("res/debug/heatmapG.png");
	config.textureFiles.push_back("res/debug/heatmap.png");
	config.textureFiles.push_back("res/debug/colors.png");
}

void setupDebugTools() {
	Settings::loadSettings("res/debug/debug_settings.json", false);
	WindowConfig config = windowConfig();

	int debugLayer = UpdateList::getLayerCount();
	LayerData &data = UpdateList::getLayerData(debugLayer);
	data.name = "DEBUG";
	data.global = true;

	int debugTextures = std::distance(config.textureFiles.begin(),
		std::find(config.textureFiles.begin(), config.textureFiles.end(), "_DEBUG"));

	UpdateList::addUNode(new DebugLayers(debugLayer));

	debugBreakpoints = new DebugBreakpoints(debugLayer);
	UpdateList::addUNode(debugBreakpoints);

	new ImguiFPS(debugLayer);
	new ImguiNodes(debugTextures+3, debugLayer);
	new ImguiEvents(debugLayer);
	new ImguiResources(debugTextures+3, debugLayer);
	new ImguiSettings(debugLayer);
	new ImguiNoiseGen(debugTextures+1, debugLayer);
}