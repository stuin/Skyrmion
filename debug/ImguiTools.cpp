#include "../core/UpdateList.h"
#include "../include/imgui/imgui.h"//

bool layersWindow = false;
bool layersShown[MAXLAYER];

bool nodeTreeWindow = false;

struct LayerProxy {
	int layer;
	bool hidden, paused, stat;
	bool _hidden, _paused, _stat;

	LayerProxy(int _layer) {
		layer = _layer;
		hidden = UpdateList::isLayerHidden(layer);
		paused = UpdateList::isLayerPaused(layer);
		stat = UpdateList::isLayerStatic(layer);
		_hidden = hidden;
		_paused = paused;
		_stat = stat;
	}

	void update() {
		if(_hidden == hidden)
			hidden = UpdateList::isLayerHidden(layer);
		else
			UpdateList::hideLayer(layer, hidden);
		_hidden = hidden;

		if(_paused == paused)
			paused = UpdateList::isLayerPaused(layer);
		else
			UpdateList::pauseLayer(layer, paused);
		_paused = paused;

		if(_stat == stat)
			stat = UpdateList::isLayerStatic(layer);
		else
			UpdateList::staticLayer(layer, stat);
		_stat = stat;
	}
};

std::vector<std::string> _layerNames = layerNames();
std::vector<LayerProxy> layerProxy;

void skyrmionImguiMenu() {
	if(ImGui::BeginMenu("Skyrmion")) {
		ImGui::MenuItem("Layers", 0, &layersWindow);
		ImGui::MenuItem("Node Trees", 0, &nodeTreeWindow);
		ImGui::EndMenu();
	}
}

void skyrmionImguiLayersWindow() {
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Layers", &layersWindow);

    for(int layer = 0; layer < _layerNames.size(); layer++) {
    	ImGui::PushID(layer);
    	if(layer >= layerProxy.size())
    		layerProxy.emplace_back(layer);

    	if(ImGui::CollapsingHeader(_layerNames[layer].c_str())) {
    		ImGui::Checkbox("Hidden", &layerProxy[layer].hidden);
    		ImGui::Checkbox("Paused", &layerProxy[layer].paused);
    		ImGui::Checkbox("Static", &layerProxy[layer].stat);
    		layerProxy[layer].update();

    		int count = 0;
    		Node *source = UpdateList::getNode(layer);
    		while(source != NULL) {
    			count++;
				source = source->getNext();
			}
			ImGui::Text("%d Nodes", count);
    	}
    	ImGui::PopID();
    }

    ImGui::End();
}

void skyrmionImguiNodeTreeWindow() {
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Node Trees", &nodeTreeWindow);

    ImGui::End();
}

void skyrmionImgui() {
	if(layersWindow)
		skyrmionImguiLayersWindow();
	if(nodeTreeWindow)
		skyrmionImguiNodeTreeWindow();
}