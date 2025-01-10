#include "../core/UpdateList.h"
#include "../input/InputHandler.h"

#include "DebugLayers.hpp"
#include "DebugBreakpoints.hpp"

void setupDebugTools() {
	Settings::loadSettings("src/Skyrmion/res/debug_settings.json");

	int debugLayer = UpdateList::getMaxLayer()+1;
	UpdateList::staticLayer(debugLayer);

	UpdateList::addNode(new DebugLayers(debugLayer));

	debugBreakpoints = new DebugBreakpoints(debugLayer);
	UpdateList::addNode(debugBreakpoints);
}


#if _DEBUG
	#define DEBUGTOOLS setupDebugTools()
#else
	#define DEBUGTOOLS void
#endif