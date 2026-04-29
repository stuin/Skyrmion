#include "../core/UpdateList.h"
#include "../util/TimingStats.hpp"

#include "../include/imgui/imgui.h"//

class ImguiFPS : public UNode {
private:
	bool open = false;

public:
	ImguiFPS(int debugLayer) : UNode(debugLayer) {
		UpdateList::addUNode(this);
		UpdateList::addListener(this, EVENT_IMGUI);
	}

	void timer(TimingStats stats, bool theoretical) {
		if(theoretical)
			ImGui::Text("Theoretical Per Second = %d", stats.getFPS());
		else
			ImGui::Text("Per Second = %d", stats.getFPS());

	    ImGui::Text("Last delta = %f", stats.last());
	    ImGui::Text("Average delta = %f", stats.totalTime/stats.totalCount);
	    ImGui::Text("Max delta = %f", stats.maxDelta);

	    if(!theoretical)
	    	ImGui::Text("Total count = %d", stats.totalCount);
	    ImGui::Text("Total time = %f", stats.totalTime);
	}

	void showWindow() {
		ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
	    ImGui::Begin("FPS", &open);

	    #ifndef PLATFORM_WEB
		    ImGui::SeparatorText("Updates");
		    timer(DebugTimers::updateTimes, false);

		    ImGui::SeparatorText("Literal Update Length");
		    timer(DebugTimers::updateLiteralTimes, true);
		#endif

	    ImGui::SeparatorText("Draw Frames");
	    timer(DebugTimers::frameTimes, false);

	    ImGui::SeparatorText("Draw Only Nodes");
	    timer(DebugTimers::frameNodeTimes, true);

	    ImGui::SeparatorText("Draw Only Buffers");
	    ImGui::Text("Max delta = %f", DebugTimers::frameBufferTimes.maxDelta);
	    ImGui::Text("Total count = %d", DebugTimers::frameBufferTimes.totalCount);
	    ImGui::Text("Total time = %f", DebugTimers::frameBufferTimes.totalTime);

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