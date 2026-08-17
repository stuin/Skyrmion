#include <set>

#include "../core/UpdateList.h"
#include "../include/imgui/imgui.h"//

class ImguiNodes : public UNode {
private:
	bool open = false;

	std::set<UNode *> nodeWindows;

	Node nodeCursor;
	Node rectCursor;
	int currentRect = -1;
	Node *currentRectNode = NULL;
	float pickColor[4] = { 1.0f, 0.0f, 0.2f, 1.0f };
	bool flipRects = false;

public:
	ImguiNodes(int debugLayer) : UNode(debugLayer),
	nodeCursor(debugLayer, RENDER_COLOR_RECT), rectCursor(debugLayer, RENDER_COLOR_RECT) {

		UpdateList::addUNode(this);
		UpdateList::addListener(this, EVENT_IMGUI);

		nodeCursor.setColor(skColor(239,131,68));
		rectCursor.setColor(skColor(223,135,0));
		nodeCursor.setHidden(true);
		rectCursor.setHidden(true);
		rectCursor.setOrigin(0,0);
		UpdateList::addNode(&nodeCursor);
		UpdateList::addNode(&rectCursor);
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

		for(sint layer = 0; layer < UpdateList::getLayerCount(); layer++) {
			ImGui::PushID(layer);

			LayerData &layerData = UpdateList::getLayerData(layer);
			if(ImGui::CollapsingHeader(layerData.name.c_str())) {
				ImGui::Checkbox("Hidden", &layerData.hidden);
				ImGui::Checkbox("Paused", &layerData.paused);
				ImGui::Checkbox("Global Update", &layerData.global);
				ImGui::Checkbox("Screen Position", &layerData.screen);

				ImGui::Text("%d Nodes", layerData.count);

				if(ImGui::BeginChild("##", ImVec2(400.0f, std::min(200.0f, layerData.count*20.f+10)), ImGuiChildFlags_Borders, 0)) {
					UNode *source = layerData.root;
					while(source != NULL) {
						sint id = source->getId();

						std::string nodeName = std::to_string(id);
						bool window = nodeWindows.contains(source);
						if(ImGui::Selectable(nodeName.c_str(), &window)) {
							if(window && !nodeWindows.contains(source))
								nodeWindows.insert(source);
							else if(!window && nodeWindows.contains(source))
								nodeWindows.erase(source);
						}

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
		std::string nodeName = "Node " + std::to_string(id) + " : " + UpdateList::getLayerData(source->getLayer()).name;
		bool window = true;

		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetCursorScreenPos().x+510, ImGui::GetCursorScreenPos().y), ImGuiCond_FirstUseEver);
		ImGui::Begin(nodeName.c_str(), &window);
		//if(!window)
		//	nodeWindows.erase((UNode*) source);

		bool focused = ImGui::IsWindowFocused();

		if(source->getParent() != NULL) {
			Node *parent = source->getParent();
			sint pid = parent->getId();
			std::string parentName = "Parent = " + std::to_string(pid);

			bool parentWindow = nodeWindows.contains(parent);
			if(ImGui::Selectable(parentName.c_str(), &parentWindow)) {
				if(parentWindow && !nodeWindows.contains(parent))
					nodeWindows.insert(parent);
				else if(!parentWindow && nodeWindows.contains(parent))
					nodeWindows.erase(parent);
			}
		} else
			ImGui::Text("Parent = NULL");

		Text("Position", source->getPosition());
		Text("Global Position", source->getGPosition());
		Text("Origin", source->getOrigin());
		Text("Size", source->getSize());
		Text("Scale", source->getScale());

		//Display node borders
		if(focused) {
			nodeCursor.setSize((Vector2f)source->getSize());
			nodeCursor.setOrigin(source->getSOrigin());
			nodeCursor.setPosition(source->getGPosition());
			//rectCursor.setOrigin(source->getSOrigin());
			//nodeCursor.setTextureRect({source->getSOrigin().x,source->getSOrigin().y,1,1, 22,8,1,1,0}, 4);
			//nodeCursor.createPixelRect(FloatRect(0,0, source->getSize().x,source->getSize().y), Vector2i(18,8), 0);
		}
		nodeCursor.setHidden(false);

		if(source->getRenderComponent() == NULL) {
			ImGui::Text("RenderComponent = NULL");
			ImGui::End();
			rectCursor.setHidden(true);
			return;
		}

		RenderComponent *renderer = source->getRenderComponent();
		ImGui::Text("RenderComponent = %s", RENDER_TYPE_NAMES[renderer->getType()].c_str());
		ImGui::Text("BlendMode = %s", BLENDMODE_NAMES[source->getBlendMode()].c_str());

		sint texture = source->getTexture();
		if(texture < UpdateList::getResourceCount())
			ImGui::Text("Texture = %ld (%s)", texture, UpdateList::getResourceData(texture).filename.c_str());
		else
			ImGui::Text("Texture = %ld", texture);

		if(renderer->getType() == RENDER_STRING)
			ImGui::Text("String = \"%s\"", source->getString());

		bool nodeHidden = source->isHidden();
		ImGui::Text("Hidden = ");
		ImGui::SameLine();
		ImGui::Checkbox("##", &nodeHidden);
		source->setHidden(nodeHidden);

		if(renderer->getTextureRects() != NULL && renderer->getTextureRects()->size() > 0) {
			ImGui::Text("Texture Rects = %lu", renderer->getTextureRects()->size());
			ImGui::Checkbox("Flip Rects", &flipRects);

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
						rectCursor.setSize(rect.p().size().abs()*source->getScale().abs());
						Vector2f pos = rect.p().pos()*source->getScale().abs();
						if(flipRects)
							pos.y = source->getRect().height-pos.y;
						rectCursor.setPosition(source->getGPosition()+pos);
						rectCursor.setHidden(false);
						//nodeCursor.createPixelRect(FloatRect(rect.p().pos()*source->getScale().abs(), rect.p().size()*source->getScale().abs()), Vector2i(18,13), 5);
						currentRect = rectId;
						currentRectNode = source;
					} else if(rectBorderBox) {
						currentRect = rectId;
						currentRectNode = source;
						rectCursor.setHidden();
					} else if(rectBorder) {
						currentRect = -1;
						currentRectNode = NULL;
						rectCursor.setHidden();
					}

					rectId++;
					ImGui::PopID();
				}
			}
			ImGui::EndChild();
		} else if(renderer->getType() == RENDER_TEXTURE_RECT) {
			TextureRect rect = *renderer->getTextureRect();
			ImGui::Text("(%.3f,%.3f) = (%d,%d)->(%d,%d) / %d",
						rect.px, rect.py, rect.tx, rect.ty, rect.tx+rect.twidth, rect.ty+rect.theight, rect.rotation);
			rectCursor.setHidden();
		} else {
			ImGui::Text("No Texture Rects");
			rectCursor.setHidden();
		}
		if(renderer->getColors() != NULL && renderer->getColors()->size() > 0) {
			ImGui::Text("Colors = %lu", renderer->getColors()->size());

			if(ImGui::BeginChild("##", ImVec2(400.0f, 200.0f), ImGuiChildFlags_Borders, 0)) {
				focused |= ImGui::IsWindowFocused();

				int colorId = 0;
				for(skColor color : *renderer->getColors()) {
					std::string id = "##" + std::to_string(colorId++);
					color.write(pickColor);
					if(ImGui::ColorEdit3(id.c_str(), pickColor))
						renderer->setColor(skColor3(pickColor), colorId);
				}
			}
			ImGui::EndChild();
		}

		ImGui::End();
	}

	void recieveEvent(Event event) {
		if(event.type == EVENT_IMGUI && event.down) {
			ImGui::MenuItem("Layers", 0, &open);
		} else if(event.type == EVENT_IMGUI) {
			if(open)
				showWindow();

			nodeCursor.setHidden(true);
			for(auto it = nodeWindows.begin() ; it != nodeWindows.end();it++) {
				if(!(*it)->isDeleted())
					showNodeWindow((Node*)*it);
				else
					nodeWindows.erase(it);
			}
		}
	}
};