#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "GridMaker.h"
#include "../util/VertexGraph.hpp"
#include "../core/Node.h"

class GridSection : public Vertex<4>, public Node {
public:
	sint id;
	std::string file;
	int tileOffset = 0;

	sint upId = 0;
	sint rightId = 0;
	sint downId = 0;
	sint leftId = 0;

	sint width = 0;
	sint height = 0;
	int x = 0;
	int y = 0;

	GridSection(GridSection *root, json data, Layer layer) : Vertex(root), Node(layer) {
		id = data.value("id", 0);
		file = data.value("file", "");
		tileOffset = data.value("tile_offset", 0);

		upId = data.value("up", 0);
		rightId = data.value("right", 0);
		downId = data.value("down", 0);
		leftId = data.value("left", 0);

		x = data.value("x_offset", 0);
		y = data.value("y_offset", 0);

		std::string line;
		std::ifstream mapFile(file);

		//Get maximum file size
		while(std::getline(mapFile, line)) {
			if(line.size() > width)
				width = line.size();
			++height;
		}
		mapFile.close();

		setSize(Vector2i(width, height));
		createPixelRect(FloatRect(0,0, width, height), Vector2i(0,0), 0);
	}

	void updateSize(Vector2i scale) {
		setPosition((x + width/2.0) * scale.x, (y + height/2.0) * scale.y);
		setSize(Vector2i(width * scale.x, height * scale.y));
		createPixelRect(FloatRect(0,0, width * scale.x, height * scale.y), Vector2i(0,0), 0);
	}
};

class GridSectioner {
	std::vector<GridSection *> sections;
	GridSection *root;
	json world;

	int x = 0;
	int y = 0;

public:
	GridMaker *grid;
	sint width = 0;
	sint height = 0;

	GridSectioner(std::string file, Layer _layer, Vector2i scale, std::function<GridSection*(GridSection *root, json data, Layer layer)> factory) {
		std::ifstream f(file);
		world = json::parse(f);
		root = factory(NULL, world["maps"][0], _layer);
		sections.push_back(root);
		width = root->width;
		height = root->height;

		GridSection *next;
		for(sint i = 1; i < world["maps"].size(); i++) {
			next = factory(root, world["maps"][i], _layer);
			int id = world["maps"][i].value("id", -1);

			for(sint j = sections.size() - 1; j < (sint)id; j++)
				sections.push_back(NULL);

			if(id < 0)
				std::cout << "Section missing id\n";
			else if(sections[id] != NULL)
				std::cout << "Duplicate section id\n";
			else
				sections[id] = next;
		}

		readNeighbors(0, root);
		//root->printAddress();
		width -= x;
		height -= y;

		grid = new GridMaker(width, height);
		for(sint i = 0; i < sections.size(); i++) {
			next = sections[i];
			if(next != NULL) {
				next->x -= x;
				next->y -= y;
				next->updateSize(scale);

				//std::cout << next->file << " " << next->tileOffset << "\n";
				//std::cout << next->x << "," << next->y << "," << next->width << "," << next->height << "\n";
				grid->reload(next->file, next->tileOffset, Rect<int>(next->x, next->y, next->width, next->height));
			}
		}
		//grid->printGrid();
	}

	void readNeighbors(int i, GridSection *prev) {
		GridSection *next;
		x = std::min(x, prev->x);
		y = std::min(y, prev->y);

		if(prev->upId != 0 && !prev->hasEdge(UP/2) && prev->upId < sections.size()) {
			next = sections[prev->upId];
			if(next != NULL) {
				root->addVertex(UP/2, next, DOWN/2);

				next->x += prev->x;
				next->y += prev->y - next->height;
				y = std::min(y, next->y);

				readNeighbors(prev->upId, next);
			}
		}
		if(prev->rightId != 0 && !prev->hasEdge(RIGHT/2) && prev->rightId < sections.size()) {
			next = sections[prev->rightId];
			if(next != NULL) {
				root->addVertex(RIGHT/2, next, LEFT/2);

				next->x += prev->x + prev->width;
				next->y += prev->y;
				width = std::max(width, next->x + next->width);

				readNeighbors(prev->rightId, next);
			}
		}
		if(prev->downId != 0 && !prev->hasEdge(DOWN/2) && prev->downId < sections.size()) {
			next = sections[prev->downId];
			if(next != NULL) {
				root->addVertex(DOWN/2, next, UP/2);

				next->x += prev->x;
				next->y += prev->y + prev->height;
				height = std::max(height, next->y + next->height);

				readNeighbors(prev->downId, next);
			}
		}
		if(prev->leftId != 0 && !prev->hasEdge(LEFT/2) && prev->leftId < sections.size()) {
			next = sections[prev->leftId];
			if(next != NULL) {
				root->addVertex(LEFT/2, next, RIGHT/2);
				std::cout << "left " << next->width << "\n";

				next->x += prev->x - next->width;
				next->y += prev->y;
				x = std::min(x, next->x);

				readNeighbors(prev->leftId, next);
			}
		}
	}

	std::vector<Node *> getNodes() {
        std::vector<Node *> nodes;
        for(GridSection *section : sections)
        	if(section != NULL)
            	nodes.push_back(section);
        return nodes;
    }
};