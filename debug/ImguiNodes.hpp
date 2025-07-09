#include "../core/UpdateList.h"

#include "../include/imgui/imgui.h"//

class ImguiNodes : public Node {
private:
	bool open = false;

	std::vector<bool> nodeWindows;
	std::vector<Node *> nodes;

	Node *debugCursor = NULL;
	int currentRect = -1;
	Node *currentRectNode = NULL;

public:
	ImguiNodes(int _pickTexture, int debugLayer) : Node(debugLayer, Vector2i(16, 16), true) {
		UpdateList::addNode(this);
		UpdateList::addListener(this, EVENT_IMGUI);

		debugCursor = new Node(debugLayer, Vector2i(1, 1), true);
		debugCursor->setTexture(_pickTexture);
		UpdateList::addNode(debugCursor);

		//Initial nodes and layer names
		for(Layer layer = 0; layer < layerNames().size(); layer++) {
			Node *source = UpdateList::getNode(layer);
			while(source != NULL) {
				sint id = source->getId();
				while(id >= nodeWindows.size()) {
					nodeWindows.push_back(false);
					nodes.push_back(NULL);
				}
				nodes[id] = source;
				source = source->getNext();
			}
		}
	}

	void Text(std::string name, Vector2f value) {
		name += " = (%.3f,%.3f)";
		ImGui::Text(name.c_str(), value.x, value.y);
	}

	void Text(std::string name, Vector2i value) {
		name += " = (%d,%d)";
		ImGui::Text(name.c_str(), value.x, value.y);
	}

	void showWindow() {
		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin("Layers", &open);

		for(Layer layer = 0; layer < UpdateList::getLayerCount(); layer++) {
			ImGui::PushID(layer);

			LayerData &layerData = UpdateList::getLayerData(layer);
			if(ImGui::CollapsingHeader(layerData.name.c_str())) {
				ImGui::Checkbox("Hidden", &layerData.hidden);
				ImGui::Checkbox("Paused", &layerData.paused);
				ImGui::Checkbox("Global Update", &layerData.global);

				ImGui::Text("%d Nodes", layerData.count);

				if(ImGui::BeginChild("##", ImVec2(400.0f, std::min(200.0f, layerData.count*20.f+10)), ImGuiChildFlags_Borders, 0)) {
					Node *source = layerData.root;
					while(source != NULL) {
						sint id = source->getId();
						while(id >= nodeWindows.size()) {
							nodeWindows.push_back(false);
							nodes.push_back(NULL);
						}
						nodes[id] = source;

						std::string nodeName = std::to_string(id);
						bool window = nodeWindows[id];
						if(ImGui::Selectable(nodeName.c_str(), &window))
							nodeWindows[id] = window;
						source = source->getNext();
					}
				}
				ImGui::EndChild();
			}
			ImGui::PopID();
		}

		ImGui::End();
	}

	void showNodeWindow(Node *source) {
		sint id = source->getId();
		std::string nodeName = "Node " + std::to_string(id) + " : " + layerNames()[source->getLayer()];
		bool window = nodeWindows[id];

		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetCursorScreenPos().x+510, ImGui::GetCursorScreenPos().y), ImGuiCond_FirstUseEver);
		ImGui::Begin(nodeName.c_str(), &window);
		nodeWindows[id] = window;

		bool focused = ImGui::IsWindowFocused();

		if(source->getParent() != NULL) {
			sint pid = source->getParent()->getId();
			std::string parentName = "Parent = " + std::to_string(pid);

			bool parentWindow = nodeWindows[pid];
			if(ImGui::Selectable(parentName.c_str(), &parentWindow))
				nodeWindows[pid] = parentWindow;
		} else
			ImGui::Text("Parent = NULL");

		Text("Position", source->getPosition());
		Text("Origin", source->getOrigin());
		Text("Size", source->getSize());
		Text("Scale", source->getGScale());

		ImGui::Text("BlendMode = %d", source->getBlendMode());

		sint texture = source->getTexture();
		if(texture < textureFiles().size())
			ImGui::Text("Texture = %ld (%s)", texture, textureFiles()[texture].c_str());
		else
			ImGui::Text("Texture = %ld", texture);

		bool nodeHidden = source->isHidden();
		ImGui::Text("Hidden = ");
		ImGui::SameLine();
		ImGui::Checkbox("##", &nodeHidden);
		source->setHidden(nodeHidden);

		if(source->getTextureRects()->size() > 0) {
			ImGui::Text("Texture Rects = %lu", source->getTextureRects()->size());

			if(ImGui::BeginChild("##", ImVec2(400.0f, 200.0f), ImGuiChildFlags_Borders, 0)) {
				focused |= ImGui::IsWindowFocused();

				int rectId = 0;
				for(TextureRect rect : *source->getTextureRects()) {
					bool rectBorder = source == currentRectNode && rectId == currentRect;
					bool rectBorderBox = rectBorder;
					ImGui::PushID(rectId);
					ImGui::Checkbox("##", &rectBorderBox);
					ImGui::SameLine();
					ImGui::Text("(%.3f,%.3f) = (%d,%d)->(%d,%d) / %d",
						rect.px, rect.py, rect.tx, rect.ty, rect.tx+rect.twidth, rect.ty+rect.theight, rect.rotation);

					if(rectBorderBox && focused) {
						debugCursor->createPixelRect(FloatRect(rect.p().pos()*source->getGScale().abs(), rect.p().size()*source->getGScale().abs()), Vector2i(18,13), 5);
						currentRect = rectId;
						currentRectNode = source;
					} else if(rectBorderBox) {
						currentRect = rectId;
						currentRectNode = source;
						debugCursor->getTextureRects()->resize(5);
					} else if(rectBorder) {
						currentRect = -1;
						currentRectNode = NULL;
						debugCursor->getTextureRects()->resize(5);
					}

					rectId++;
					ImGui::PopID();
				}
			}
			ImGui::EndChild();
		} else
			ImGui::Text("No Texture Rects");

		//Display node borders
		if(focused) {
			debugCursor->setSize((Vector2f)source->getSize());
			debugCursor->setOrigin(source->getSOrigin());
			debugCursor->setTextureRect({source->getSOrigin().x,source->getSOrigin().y,1,1, 22,8,1,1,0}, 4);
			debugCursor->createPixelRect(FloatRect(0,0, source->getSize().x,source->getSize().y), Vector2i(18,8), 0);
			debugCursor->setPosition(source->getGPosition());
		}
		debugCursor->setHidden(false);

		ImGui::End();
	}

	void recieveEvent(Event event) {
		if(event.type == EVENT_IMGUI && event.down) {
			ImGui::MenuItem("Layers", 0, &open);
		} else if(event.type == EVENT_IMGUI) {
			if(open)
				showWindow();

			debugCursor->setHidden(true);
			for(sint i = 0; i < nodeWindows.size(); i++)
				if(nodeWindows[i] && nodes[i] != NULL && !nodes[i]->isDeleted())
					showNodeWindow(nodes[i]);
				else if(nodes[i] != NULL && nodes[i]->isDeleted()) {
					nodes[i] = NULL;
					nodeWindows[i] = false;
				}
		}
	}
};