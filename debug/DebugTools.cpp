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

void setupDebugTools() {
	SETTINGS.loadSettings("res/debug/debug_settings.json", false);

	int debugLayer = UpdateList::getLayerCount();
	LayerData &data = UpdateList::getLayerData(debugLayer);
	data.name = "DEBUG";
	data.global = true;

	UpdateList::addUNode(new DebugLayers(debugLayer));

	debugBreakpoints = new DebugBreakpoints(debugLayer);
	UpdateList::addUNode(debugBreakpoints);

	new ImguiFPS(debugLayer);
	new ImguiNodes(debugLayer);
	new ImguiEvents(debugLayer);
	new ImguiResources(debugLayer);
	new ImguiSettings(debugLayer);
	new ImguiNoiseGen(debugLayer);
}