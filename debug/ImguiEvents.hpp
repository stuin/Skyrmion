#include "../core/UpdateList.h"

#include "../include/imgui/imgui.h"//

static std::vector<std::string> EVENT_NAME = {
	"KEYPRESS", "MOUSE", "SCROLL", "TOUCH", "JOYSTICK", "JOYSTICK_SIM",
	"RESIZE", "FOCUS", "SUSPEND", "IMGUI",
	"NETWORK_CONNECT_SERVER", "NETWORK_CONNECT_CLIENT", "NETWORK_POSITION", "NETWORK",
	"CUSTOM", "MAX"
};

class ImguiEvents : public Node {
private:
	bool open = false;

	Event last[EVENT_MAX];
	double since[EVENT_MAX] = {0};
	sint count[EVENT_MAX] = {0};

public:
	ImguiEvents(int debugLayer) : Node(debugLayer, Vector2i(16, 16), true) {
		UpdateList::addNode(this);

		for(int i = 0; i < EVENT_MAX; i++)
			UpdateList::addListener(this, i);
	}

	void showWindow() {
		ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
	    ImGui::Begin("Events", &open);

	    for(int i = 0; i < EVENT_MAX; i++) {
	    	ImGui::PushID(i);

	    	if(ImGui::CollapsingHeader(EVENT_NAME[i].c_str())) {
	    		ImGui::Text("type = %d", last[i].type);
	    		ImGui::Text("down = %d", last[i].down);
	    		ImGui::Text("code = %d", last[i].code);
	    		ImGui::Text("x,y = (%.3f,%.3f)", last[i].x, last[i].y);

	    		ImGui::Text("number recieved = %ld", count[i]);
	    		ImGui::Text("time since = %.3f", since[i]);
			}
			ImGui::PopID();
	    }

	    ImGui::End();
	}

	void update(double time) {
		for(int i = 0; i < EVENT_MAX; i++)
			since[i] += time;
	}

	void recieveEvent(Event event) {
		if(event.type == EVENT_IMGUI && event.down) {
			ImGui::MenuItem("Events", 0, &open);
		} else if(event.type == EVENT_IMGUI && open) {
			showWindow();
		}

		last[event.type] = event;
		since[event.type] = 0;
		count[event.type]++;
	}
};