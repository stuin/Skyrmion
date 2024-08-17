#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "GridMaker.h"
#include "VertexGraph.hpp"

enum sides {
	UP,
	RIGHT,
	DOWN,
	LEFT
};

class GridSection : public Vertex<4> {
	int id;

public:
	std::string file;
	int tileOffset = 0;

	int upId = 0;
	int rightId = 0;
	int downId = 0;
	int leftId = 0;

	int width = 0;
	int height = 0;
	int x = 0;
	int y = 0;

	GridSection(GridSection *root, json data) : Vertex(root) {
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
	}
};

class GridManager {
	std::vector<GridSection *> sections;
	GridSection *root;
	json world;

	int width = 0;
	int height = 0;
	int x = 0;
	int y = 0;

public:
	GridMaker *grid;

	GridManager(std::string file) {
		std::ifstream f(file);
		world = json::parse(f);
		root = new GridSection(NULL, world["maps"][0]);
		sections.push_back(root);
		width = root->width;
		height = root->height;

		GridSection *next;
		for(int i = 1; i < world["maps"].size(); i++) {
			next = new GridSection(root, world["maps"][i]);
			sections.push_back(next);
		}

		readNeighbors(0, root);
		//root->printAddress();
		//std::cout << "\n" << width-x << ", " << height-y << "\n";

		grid = new GridMaker(width-x, height-y);
		for(int i = 0; i < sections.size(); i++) {
			next = sections[i];
			//std::cout << next->file << " " << next->tileOffset << "\n";
			//std::cout << next->x-x << "," << next->y-y << "," << next->width << "," << next->height << "\n";
			grid->reload(next->file, next->tileOffset, next->x-x, next->y-y, next->width, next->height);
		}
		//grid->printGrid();
	}

	void readNeighbors(int i, GridSection *prev) {
		GridSection *next;
		if(prev->upId != 0 && !prev->hasEdge(UP)) {
			next = sections[prev->upId];
			root->addVertex(UP, next, DOWN);

			next->x += prev->x;
			next->y += prev->y - next->height;
			y = std::min(y, next->y);

			readNeighbors(prev->upId, next);
		}
		if(prev->rightId != 0 && !prev->hasEdge(RIGHT)) {
			next = sections[prev->rightId];
			root->addVertex(RIGHT, next, LEFT);

			next->x += prev->x + prev->width;
			next->y += prev->y;
			width = std::max(width, next->x + next->width);

			readNeighbors(prev->rightId, next);
		}
		if(prev->downId != 0 && !prev->hasEdge(DOWN)) {
			next = sections[prev->downId];
			root->addVertex(DOWN, next, UP);

			next->x += prev->x;
			next->y += prev->y + prev->height;
			height = std::max(height, next->y + next->height);

			readNeighbors(prev->downId, next);
		}
		if(prev->leftId != 0 && !prev->hasEdge(LEFT)) {
			next = sections[prev->leftId];
			root->addVertex(LEFT, next, RIGHT);

			next->x += prev->x - next->width;
			next->y += prev->y;
			x = std::min(x, next->x);

			readNeighbors(prev->leftId, next);
		}
	}
};