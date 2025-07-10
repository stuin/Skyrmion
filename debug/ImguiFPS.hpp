#include "../core/UpdateList.h"
#include "../util/TimingStats.hpp"

#include "../include/imgui/imgui.h"//

class ImguiFPS : public Node {
private:
	bool open = false;

public:
	ImguiFPS(int debugLayer) : Node(debugLayer, Vector2i(16, 16), true) {
		UpdateList::addNode(this);
		UpdateList::addListener(this, EVENT_IMGUI);
	}

	void showWindow() {
		ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
	    ImGui::Begin("FPS", &open);

	    #ifndef PLATFORM_WEB
		    ImGui::SeparatorText("Updates");
		    ImGui::Text("Per Second = %d", DebugTimers::updateTimes.getFPS());
		    ImGui::Text("Last delta = %f", DebugTimers::updateTimes.last());
		    ImGui::Text("Average delta = %f", DebugTimers::updateTimes.totalTime/DebugTimers::updateTimes.totalCount);
		    ImGui::Text("Max delta = %f", DebugTimers::updateTimes.maxDelta);
		    ImGui::Text("Total updates = %d", DebugTimers::updateTimes.totalCount);
		    ImGui::Text("Total time = %f", DebugTimers::updateTimes.totalTime);

		    ImGui::SeparatorText("Literal Update Length");
		    ImGui::Text("Theoretical Per Second = %d", DebugTimers::updateLiteralTimes.getFPS());
		    ImGui::Text("Last delta = %f", DebugTimers::updateLiteralTimes.last());
		    ImGui::Text("Average delta = %f", DebugTimers::updateLiteralTimes.totalTime/DebugTimers::updateLiteralTimes.totalCount);
		    ImGui::Text("Max delta = %f", DebugTimers::updateLiteralTimes.maxDelta);
		    ImGui::Text("Total time = %f", DebugTimers::updateLiteralTimes.totalTime);
		#endif

	    ImGui::SeparatorText("Draw Frames");
	    ImGui::Text("Per Second = %d", DebugTimers::frameTimes.getFPS());
	    ImGui::Text("Last delta = %f", DebugTimers::frameTimes.last());
	    ImGui::Text("Average delta = %f", DebugTimers::frameTimes.totalTime/DebugTimers::frameTimes.totalCount);
	    ImGui::Text("Max delta = %f", DebugTimers::frameTimes.maxDelta);
	    ImGui::Text("Total frames = %d", DebugTimers::frameTimes.totalCount);
	    ImGui::Text("Total time = %f", DebugTimers::frameTimes.totalTime);

	    ImGui::SeparatorText("Draw Only Nodes");
	    ImGui::Text("Theoretical Per Second = %d", DebugTimers::frameLiteralTimes.getFPS());
	    ImGui::Text("Last delta = %f", DebugTimers::frameLiteralTimes.last());
	    ImGui::Text("Average delta = %f", DebugTimers::frameLiteralTimes.totalTime/DebugTimers::frameLiteralTimes.totalCount);
	    ImGui::Text("Max delta = %f", DebugTimers::frameLiteralTimes.maxDelta);
	    ImGui::Text("Total time = %f", DebugTimers::frameLiteralTimes.totalTime);

	    ImGui::End();
	}

	void recieveEvent(Event event) {
		if(event.type == EVENT_IMGUI && event.down) {
			ImGui::MenuItem("FPS", 0, &open);
		} else if(event.type == EVENT_IMGUI && open) {
			showWindow();
		}
	}
};