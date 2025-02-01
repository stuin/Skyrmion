#include "../tiling/GridMaker.h"
#include "../core/UpdateList.h"

#include "../include/imgui/imgui.h"//


class GridEditor : public Node {
private:
	bool open = false;
	int current = 0;
	Vector2i tileSize;

	std::string name;
	GridMaker *grid;
	std::map<int, std::string> tiles;

public:
	GridEditor(std::string _name, GridMaker *_grid, std::map<int, std::string> _tiles, Vector2i size, Layer layer) : Node(layer, size),
	name(_name), grid(_grid), tiles(_tiles) {
		UpdateList::addNode(this);
		UpdateList::addListener(this, EVENT_IMGUI);
		UpdateList::addListener(this, EVENT_MOUSE);

		current = tiles.begin()->first;
		tileSize = Vector2i(size/_grid->getSize());
		setTextureRect({0});
	}

	void showWindow() {
		ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
    	ImGui::Begin(name.c_str(), &open);

    	ImGui::Text("Tile Size = (%d,%d)", tileSize.x, tileSize.y);
    	ImGui::Text("Grid Size = (%d,%d)", getSize().x/tileSize.x, getSize().y/tileSize.y);
    	ImGui::Text("%lu Tile Types", tiles.size());
    	ImGui::Text("Selected = %c", (char)current);

    	ImGui::SeparatorText("Tiles");
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
		} else if(event.type == EVENT_IMGUI && open) {
			showWindow();
		} else if(event.type == EVENT_MOUSE && open) {
			Vector2f pos = screenToGlobal(event.x, event.y);
			if(getRect().contains(pos)) {
				pos = round(pos / tileSize) * tileSize - tileSize / 2;
				createPixelRect({pos.x, pos.y, (float)tileSize.x, (float)tileSize.y}, Vector2i(0,0), 0);

				if(event.down && !ImGui::GetIO().WantCaptureMouse) {
					pos = (pos + Vector2f(tileSize/2)) / tileSize;
					if(event.code == 0)
						grid->setTileI(pos.x, pos.y, current);
					else if(event.code == 2)
						current = grid->getTileI(pos.x, pos.y);
				}
			}
		}
	}
};

void addGridEditor(std::string name, GridMaker *grid, FloatRect bounds, std::map<int, std::string> tiles, sint texture, Layer layer) {
	GridEditor *editor = new GridEditor(name, grid, tiles, bounds.getSize(), layer);
	editor->setPosition(bounds.getPosition());
	editor->setOrigin(0,0);
	//editor->setTexture(texture);
}