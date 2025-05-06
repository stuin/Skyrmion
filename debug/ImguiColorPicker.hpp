#include "../core/UpdateList.h"

#include "../include/imgui/imgui.h"//

class ImguiColorPicker : public Node {
private:
	bool open = false;

	float col1[4] = { 1.0f, 0.0f, 0.2f, 1.0f };
	Vector2i pickPosition;
	int pickIndex = 10;
	int pickTexture = -1;
	float pickTextureScale = 6;

public:
	ImguiColorPicker(int _pickTexture, int debugLayer) : Node(debugLayer, Vector2i(16, 16), true) {
		UpdateList::addNode(this);
		UpdateList::addListener(this, EVENT_IMGUI);

		pickTexture = _pickTexture;
	}

	void showWindow() {
		ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
	    ImGui::Begin("Color Picker", &open);

	    ImGui::ColorEdit4("Color", col1);
		ImGui::SeparatorText("Image Selector");

	    int texture = pickTexture;
	    ImGui::SliderInt("Texture", &pickTexture, 0, textureFiles().size()-1);
		ImGui::SliderFloat("Scale", &pickTextureScale, 0.0f, 10.0f, "%.3f");
		TextureData &textureData = UpdateList::getTextureData(pickTexture);
		ImGui::Text("%s", textureData.filename.c_str());
	    //ImGui::Text("Buffer ID: %ld", textureData.buffer);
	    if(!textureData.valid) {
	    	ImGui::End();
	    	return;
	    }

	    ImGui::SeparatorText("Color Picker");

	    if(texture != pickTexture)
	    	pickPosition = Vector2i(0,0);

	    Vector2i size = textureData.size;
	    ImGui::SliderInt("x", &pickPosition.x, 0, size.x-1);
	    ImGui::SameLine();
	    ImGui::Text("/ %d", size.x);

	    ImGui::SliderInt("y", &pickPosition.y, 0, size.y-1);
	    ImGui::SameLine();
	    ImGui::Text("/ %d", size.y);

	    int index = pickPosition.x + pickPosition.y*size.x;
	    ImGui::SliderInt("Index", &index, 0, size.x*size.y-1);

	    if(pickIndex != index || texture != pickTexture) {
	    	pickPosition.x = index % size.x;
	    	pickPosition.y = index / size.x;

	    	skColor pickColor = UpdateList::pickColor(pickTexture, pickPosition);
		    col1[0] = pickColor.red;
		    col1[1] = pickColor.green;
		    col1[2] = pickColor.blue;
		    col1[3] = pickColor.alpha;
		    pickIndex = index;
		}

		UpdateList::drawImGuiTexture(pickTexture, Vector2i(size.x*pickTextureScale, size.y*pickTextureScale));

	    ImGui::End();
	}

	void recieveEvent(Event event) {
		if(event.type == EVENT_IMGUI && event.down) {
			ImGui::MenuItem("Color Picker", 0, &open);
		} else if(event.type == EVENT_IMGUI && open) {
			showWindow();
		}
	}
};