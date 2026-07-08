#include "../tiling/GridMaker.h"
#include "../core/UpdateList.h"

#include "../include/imgui/imgui.h"//


class GridEditor : public Node {
private:
	bool open = false;
	int current = 0;
	Vector2i tileSize;
	Node *cursor;

	int offset = 0;
	int offsetMax = 0;
	int offsetMult = 1;
	int offsetSub = 0;
	std::vector<std::string> offsetNames;

	GridMaker *grid;
	std::map<int, std::string> tiles;
	bool hexMode;

	std::function<void(int, Vector2f)> writeFunc;
	bool useFunction = false;

public:
	std::string name = "Grid Editor";
	std::string saveFile = "";
	std::string loadFile = "";

	GridEditor(int layer, GridMaker *_grid, std::map<int, std::string> _tiles, Vector2i _tileSize, Node *_cursor, bool _hexMode=false) :
		Node(layer, RENDER_NONE, _tileSize*_grid->getSize()), grid(_grid), tiles(_tiles), hexMode(_hexMode) {

		UpdateList::addListener(this, EVENT_IMGUI);
		UpdateList::addListener(this, EVENT_MOUSE);

		current = tiles.begin()->first;
		tileSize = _tileSize;

		if(_cursor == NULL) {
			cursor = new Node(layer, RENDER_COLOR_RECT, _tileSize);
			cursor->setSize(tileSize);
			cursor->setOrigin(0,0);
			UpdateList::addNode(cursor);
		} else
			cursor = _cursor;
	}

	void setFunction(std::function<void(int, Vector2f)> _func) {
		writeFunc = _func;
		useFunction = true;
	}

	void setupOffset(int _offsetMax, int _offsetMult, int _offsetSub, std::vector<std::string> _offsetNames) {
		offsetMax = _offsetMax;
		offsetMult = _offsetMax;
		offsetSub = _offsetSub;
		offsetNames = _offsetNames;
	}

	void showWindow() {
		ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
    	ImGui::Begin(name.c_str(), &open);

    	if(saveFile == loadFile)
    		ImGui::Text("Grid file = %s", loadFile.c_str());
    	else {
    		ImGui::Text("Save file = %s", saveFile.c_str());
    		ImGui::Text("Load file = %s", loadFile.c_str());
    	}

    	ImGui::Text("Tile Size = (%d,%d)", tileSize.x, tileSize.y);
    	ImGui::Text("Grid Size = (%.3f,%.3f)", getSize().x/tileSize.x, getSize().y/tileSize.y);
    	ImGui::Text("%lu Tile Types", tiles.size());
    	ImGui::Text("Selected = %c", (char)current);

    	if(ImGui::Button("Save") && saveFile != "")
    		grid->save(saveFile);
    	ImGui::SameLine();
    	if(ImGui::Button("Load") && loadFile != "")
    		grid->reload(loadFile);

    	ImGui::SeparatorText("Tiles");

    	if(offsetMax > 0) {
    		ImGui::SliderInt("Offset", &offset, 0, offsetMax);
    		ImGui::Text(offsetNames[offset].c_str());
    	}

    	if(ImGui::BeginListBox("##")) {
	    	for(auto it = tiles.begin(); it != tiles.end(); ++it) {
	    		bool selected = (it->first == current);
	    		if(ImGui::Selectable(it->second.c_str(), &selected))
	    			current = it->first;
	    	}
	    	ImGui::EndListBox();
	    }

    	ImGui::End();
	}

	void recieveEvent(Event event) {
		if(event.type == EVENT_IMGUI && event.down) {
			ImGui::MenuItem(name.c_str(), 0, &open);
		} else if(event.type == EVENT_IMGUI) {
			if(open)
				showWindow();
			cursor->setHidden(!open);
		} else if(event.type == EVENT_MOUSE && open) {
			Vector2f pos = screenToGlobal(event.x, event.y);
			if(getRect().contains(pos)) {
				pos -= getPosition();
				Vector2i tilePos = pos / tileSize;
				if(hexMode && tilePos.y%2==0) {
					if(((int)pos.x % tileSize.x) < tileSize.x/2)
						tilePos.x -= 1;
					pos = tilePos * tileSize;
					pos.x += tileSize.x/2;
				} else
					pos = tilePos * tileSize;
				pos += getPosition();
				cursor->setPosition(pos);
				//createPixelRect({pos.x, pos.y, (float)tileSize.x, (float)tileSize.y}, Vector2i(0,0), 0);

				if(event.down && !ImGui::GetIO().WantCaptureMouse) {
					if(event.code == 0) {
						int c = current + offset*offsetMult;
						grid->setTileI(tilePos.x, tilePos.y, c);
						if(useFunction)
							writeFunc(c, tilePos);
					} else if(event.code == 2) {
						current = grid->getTileI(tilePos.x, tilePos.y);
						if(offsetMax != 0) {
							offset = (current - offsetSub) / offsetMult;
							current = (current - offsetSub) % offsetMult + offsetSub;
						}
					}
				}
			}
		}
	}
};