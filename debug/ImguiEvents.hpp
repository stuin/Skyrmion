#include "../core/UpdateList.h"
#include "../input/InputHandler.h"
#include "../input/Settings.h"

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

	std::map<int, Keybind> keys;

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

				//Keyboard inputs
				if(i == EVENT_KEYPRESS) {
					if(ImGui::BeginChild("##", ImVec2(200.0f, 100.0f), ImGuiChildFlags_Borders, 0)) {
						for(auto it = keys.begin(); it != keys.end(); ++it) {
							//ImGui::PushID(it->first);
							if(it->second.pressed)
								ImGui::Text(it->second.configName.c_str());
							else
								ImGui::TextDisabled(it->second.configName.c_str());
							//ImGui::PopID();
						}
					}
					ImGui::EndChild();
				}
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
		} else if(event.type == EVENT_KEYPRESS) {
			//Update keybind list
			auto key = keys.find(event.code);
			if(key != keys.end())
				key->second.pressed = event.down;
			else {
				for(auto it = Settings::EVENT_KEYMAP.begin(); it != Settings::EVENT_KEYMAP.end(); ++it) {
					if(event.code == it->second)
						keys.emplace(event.code, Keybind(event.code, it->first));
				}
				if(!keys.contains(event.code))
					keys.emplace(event.code, Keybind(event.code, "NONE"));
				key = keys.find(event.code);
				key->second.pressed = true;
			}
		}

		last[event.type] = event;
		since[event.type] = 0;
		count[event.type]++;
	}
};