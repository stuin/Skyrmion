#include "../core/UpdateList.h"
#include "../input/Settings.h"

#include "../include/imgui/imgui.h"//

static ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_SpanAllColumns;
static ImGuiTreeNodeFlags root_flags = node_flags | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf;

class ImguiSettings : public UNode {
private:
	bool open = false;

	bool forceAlts = false;
	std::string remapping = "";

public:
	ImguiSettings(int debugLayer) : UNode(debugLayer) {
		UpdateList::addUNode(this);
		UpdateList::addListener(this, EVENT_IMGUI);
		UpdateList::addListener(this, EVENT_KEYPRESS);
	}

	void showKeyinput(std::string field, std::string first) {
		std::string key = field + "/" + first;
		std::string id = "##" + first;
		char str[128];
		std::strcpy(str, Settings::getString(key).c_str());
		ImGui::Text("%s =", first.c_str());
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-ImGui::GetFontSize()*3.9);
		if(ImGui::InputText(id.c_str(), str, 128))
			Settings::setString(key, str);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		id = "Remap" + id;
		if(ImGui::Button(id.c_str())) {
			//std::cout << "Remapping key " << key << "\n";
			UpdateList::startRemap();
			remapping = key;
		}
	}

	void showObject(std::string field) {
		for(string_pair s : Settings::listKeys(field)) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			std::string key = field + "/" + s.first;
			std::string id = "##" + s.first;
			if(s.second == "object") {
				if(ImGui::TreeNodeEx(s.first.c_str(), node_flags)) {
					showObject(key);
					ImGui::TreePop();
				}
			} else if(s.second == "string") {
				if(Settings::isKeycode(key)) {
					showKeyinput(field, s.first);
					if(forceAlts && key.find('&') == std::string::npos) {
						for(int i = 1; i <= MAXALTS; i++) {
							if(Settings::getString(key + "&" + std::to_string(i)) == "") {
								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								showKeyinput(field, s.first + "&" + std::to_string(i));
							}
						}
					}
				} else {
					char str[128];
					std::strcpy(str, Settings::getString(key).c_str());
					ImGui::Text("%s =", s.first.c_str());
					ImGui::TableNextColumn();
					ImGui::PushItemWidth(-FLT_MIN);
					if(ImGui::InputText(id.c_str(), str, 128))
						Settings::setString(key, str);
					ImGui::PopItemWidth();
				}
			} else if(s.second == "number") {
				int v = Settings::getInt(key);
				ImGui::Text("%s =", s.first.c_str());
				ImGui::TableNextColumn();
				ImGui::PushItemWidth(-FLT_MIN);
				if(ImGui::InputInt(id.c_str(), &v))
					Settings::setInt(key, v);
				ImGui::PopItemWidth();
			} else if(s.second == "boolean") {
				bool v = Settings::getBool(key);
				if(ImGui::Checkbox(s.first.c_str(), &v))
					Settings::setBool(key, v);
			}
		}
	}

	void showWindow() {
		ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
	    ImGui::Begin("Settings", &open);
	    if(ImGui::BeginTable("Table", 2)) {
	    	ImGui::TableNextRow();
			ImGui::TableNextColumn();
		    if(ImGui::TreeNodeEx("Root", root_flags)) {
			    showObject("");
			    ImGui::TreePop();
			}
			ImGui::EndTable();
		}

		ImGui::Checkbox("Add Alt Keybinds", &forceAlts);
	    if(ImGui::Button("Save"))
	    	Settings::save("");

	    ImGui::End();
	}

	void recieveEvent(Event event) {
		if(event.type == EVENT_IMGUI && event.down) {
			ImGui::MenuItem("Settings", 0, &open);
		} else if(event.type == EVENT_IMGUI && open) {
			showWindow();
		} else if(remapping != "" && event.type == EVENT_KEYPRESS && event.down) {
			//std::cout << "Set key " << remapping << " to " << Settings::reverseKeycode(event.code) << "\n";
			Settings::setString(remapping, Settings::reverseKeycode(event.code));
			remapping = "";
		}
	}
};