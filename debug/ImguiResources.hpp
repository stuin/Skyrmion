#include "../core/UpdateList.h"

#include "../include/imgui/imgui.h"//

class ImguiResources : public UNode {
private:
	bool open = false;

	float pickColor[4] = { 1.0f, 0.0f, 0.2f, 1.0f };
	Vector2i pickPosition;
	int pickIndex = 10;
	int pickTexture = -1;
	float pickTextureScale = 6;

public:
	ImguiResources(int _pickTexture, int debugLayer) : UNode(debugLayer) {
		UpdateList::addUNode(this);
		UpdateList::addListener(this, EVENT_IMGUI);

		pickTexture = _pickTexture;
	}

	void showWindow() {
		ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
		ImGui::Begin("Resources", &open);

		ImGui::SeparatorText("Image Selector");

		int texture = pickTexture;
		ImGui::SliderInt("Texture", &pickTexture, 0, UpdateList::getResourceCount()-1);
		ImGui::SliderFloat("Scale", &pickTextureScale, 0.0f, 10.0f, "%.3f");

		ResourceData &textureData = UpdateList::getResourceData(pickTexture);
		ImGui::Text("%s : %li", textureData.filename.c_str(), textureData.index);

		Vector2i size = textureData.size;
		ImGui::Text("Size = (%d, %d)", size.x, size.y);

		if(textureData.type >= 0) {
			switch(textureData.type) {
			case SK_INVALID:
				ImGui::Text("Invalid Resource");
				break;
			case SK_SHADER:
				ImGui::Text("Type = Shader");
				break;
			case SK_SHADER_UNIFORM: {
				ImGui::Text("Type = Shader Uniform");
				int sIndex = UpdateList::getUniform(pickTexture).shader;
				ImGui::Text("Shader = %d: %s", sIndex, UpdateList::getResourceData(sIndex).filename.c_str());
				ImGui::SeparatorText("Data");
				std::vector<int> &values = UpdateList::getUniform(pickTexture).values;

				if(size.x == 3) {
					//Edit uniform colors
					for(int y = 0; y < size.y; y++) {
						std::string id = "##" + std::to_string(y);
						skColor3(values, y).write(pickColor);
						if(ImGui::ColorEdit3(id.c_str(), pickColor))
							skColor3(pickColor).write(values, y);
					}
				} else {
					//Edit uniform numbers
					ImGui::BeginTable("Data", size.x);
					for(int y = 0; y < size.y; y++) {
						ImGui::TableNextRow();
						for(int x = 0; x < size.x; x++) {
							ImGui::TableNextColumn();
							std::string id = "##" + std::to_string(y) + ":" + std::to_string(x);
							ImGui::InputInt(id.c_str(), &values[y*size.x+x], 0, 255);
						}
					}
					ImGui::EndTable();
				}

				if(ImGui::Button("Save"))
					UpdateList::updateUniform(pickTexture, values);

				break; }
			case SK_FONT:
				ImGui::Text("Type = Font");
				ImGui::Text("Font size = %d", size.y);
				break;
			case SK_AUDIO:
				ImGui::Text("Type = Audio");
				break;
			case SK_TEXT:
				ImGui::Text("Type = Text");
				break;
			case SK_JSON:
				ImGui::Text("Type = JSON");
				break;
			default:
				ImGui::Text("Type = Unknown");
				break;
			}
			ImGui::End();
			return;
		}

		if(texture != pickTexture)
			pickPosition = Vector2i(0,0);

		if(ImGui::CollapsingHeader("Color Picker")) {

			ImGui::ColorEdit4("Color", pickColor);

			skColor color = skColor(pickColor);
			ImGui::Text("HSL: %d, %d%%, %d%%", (int)color.hue(),
				(int)(color.saturation()*100), (int)(color.luminance()*100));

			ImGui::SliderInt("x", &pickPosition.x, 0, size.x-1);
			ImGui::SameLine();
			ImGui::Text("/ %d", size.x);

			ImGui::SliderInt("y", &pickPosition.y, 0, size.y-1);
			ImGui::SameLine();
			ImGui::Text("/ %d", size.y);

			int index = pickPosition.x + pickPosition.y*size.x;
			ImGui::SliderInt("Index", &index, 0, size.x*size.y-1);

			if((pickIndex != index || texture != pickTexture) && size.x > 0) {
				pickPosition.x = index % size.x;
				pickPosition.y = index / size.x;

				color = UpdateList::pickColor(pickTexture, pickPosition);
				color.write(pickColor, 0);
				pickIndex = index;
			}
		}

		if(textureData.type == SK_TEXTURE)
			ImGui::SeparatorText("Texture");
		else
			ImGui::SeparatorText("Buffer");
		UpdateList::drawImGuiTexture(pickTexture, Vector2i(size.x*pickTextureScale, size.y*pickTextureScale));

		ImGui::End();
	}

	void recieveEvent(Event event) {
		if(event.type == EVENT_IMGUI && event.down) {
			ImGui::MenuItem("Resources", 0, &open);
		} else if(event.type == EVENT_IMGUI && open) {
			showWindow();
		}
	}
};