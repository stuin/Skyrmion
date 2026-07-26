#include "GridMaker.h"
#include "../core/Event.h"

#include <cstring>
#include <deque>

#include <limits>
#define FLT_MAX std::numeric_limits<float>::max()

/*
 * Generates and stores main tilemap
 */

int Indexer::mapTile(int c) {
	return c;
}

//Scale vector before retrieving value
int Indexer::getTile(Vector2f position) {
	return getTileI(position.x / getScale().x, position.y / getScale().y);
}

//Scale and set tile
void Indexer::setTile(Vector2f position, int value) {
	setTileI(position.x / getScale().x, position.y / getScale().y, value);
}

//Get tile int from previous
int Indexer::getTileI(int x, int y) {
	if(inBounds(x, y))
		return mapTile(previous->getTileI(x, y));
	return fallback;
}

//Set tile in grid
void Indexer::setTileI(int x, int y, int value) {
	if(inBounds(x, y))
		previous->setTileI(x, y, value);
}

//Get an individual bit from a specific tile
bool Indexer::getTileB(int x, int y, int place) {
	return (getTileI(x, y) >> place) & (int)1;
}

//Set an individual bit on a specific tile
void Indexer::setTileB(int x, int y, int place, bool value) {
	if(value)
		setTileI(x, y, getTileI(x, y) | ((int)1 << place));
	else
		setTileI(x, y, getTileI(x, y) & ~((int)1 << place));
}

//Run function on every square in grid
void Indexer::mapGrid(std::function<void(int, Vector2f)> func) {
	const int width = getSize().x;
	const int height = getSize().y;
	//Loop through tiles
	for(int y = 0; y < height; y++)
		for(int x = 0; x < width; x++) {
			Vector2f pos = Vector2f(x * getScale().x, y * getScale().y);
			func(getTileI(x, y), pos);
		}
}

void Indexer::setGrid(int *values) {
	for(int y = 0; y < getSize().y; y++)
		for(int x = 0; x < getSize().x; x++)
			setTileI(x, y, values[y*getSize().x+x]);
}

//Set all tiles
void Indexer::clearGrid(int value) {
	for(int y = 0; y < getSize().y; y++)
		for(int x = 0; x < getSize().x; x++)
			setTileI(x, y, value);
}

uint Indexer::getUpdateCount() {
	return previous->getUpdateCount();
}

static const Vector2i NEIGHBORS[] = {
	Vector2i(1,0), Vector2i(0,1),
	Vector2i(-1,0), Vector2i(0,-1)
};

int Indexer::setTileRecursive(int x, int y, int value) {
	int prev = getTileI(x, y);
	int count = 1;
	setTileI(x,y, value);

	if(inBounds(x+1,y) && getTileI(x+1, y) == prev)
		count += setTileRecursive(x+1,y, value);
	if(inBounds(x-1,y) && getTileI(x-1, y) == prev)
		count += setTileRecursive(x-1,y, value);
	if(inBounds(x,y+1) && getTileI(x, y+1) == prev)
		count += setTileRecursive(x,y+1, value);
	if(inBounds(x,y-1) && getTileI(x, y-1) == prev)
		count += setTileRecursive(x,y-1, value);
	return count;
}

Vector2i Indexer::getNearest(Vector2i start, int target) {
	if(getTileI(start.x, start.y) == target)
		return start;

	int distance = 1;
	while(distance/2 - start.x >= 0 || distance/2 + start.x < getSize().x ||
		distance/2 - start.y >= 0 || distance/2 + start.y < getSize().y) {

		for(int i = 0; i < distance; i++) {
			Vector2i next = Vector2i(start.x+distance-i, start.y+i);
			if(inBounds(next.x, next.y) && getTileI(next.x, next.y) == target)
				return next;
			next = Vector2i(start.x-distance+i, start.y-i);
			if(inBounds(next.x, next.y) && getTileI(next.x, next.y) == target)
				return next;
			next = Vector2i(start.x-i, start.y+distance-i);
			if(inBounds(next.x, next.y) && getTileI(next.x, next.y) == target)
				return next;
			next = Vector2i(start.x+i, start.y-distance+i);
			if(inBounds(next.x, next.y) && getTileI(next.x, next.y) == target)
				return next;
		}

		distance++;
	}
	return Vector2i(-1,-1);
}

bool Indexer::lineEmpty(Vector2i start, Vector2i end) {
	int length = vectorLength(start, end)-2;
	Vector2f dir = lengthVector(end-start, 1);

	if(!inBounds(start) || !inBounds(end))
		return false;

	Vector2f next = Vector2f(start)+dir;
	for(int i = 0; i < length; i++) {
		if(getTile(next) != 0)
			return false;
		next += dir;
	}
	return true;
}

class Cell {
public:
	Vector2i parent;
	double f;
	double g;
	double h;

	Cell() {
		parent = Vector2i(-1,-1);
		f = FLT_MAX;
		g = FLT_MAX;
		h = FLT_MAX;
	}
	Cell(Vector2i _parent, double _f, double _g=0, double _h=0) {
		parent = _parent;
		f = _f;
		g = _g;
		h = _h;
	}
};

//A* pathfinding
std::vector<Vector2i> Indexer::getPath(Vector2i start, Vector2i target, bool simplify, Indexer *debug) {
	std::vector<Vector2i> out;
	if(!inBounds(start) || !inBounds(target))
		return out;

	if(start == target) {
		out.push_back(start);
		return out;
	}

	//std::cout << start << " to " << target << "\n";

	bool closed[getSize().x][getSize().y];
	memset(closed, false, sizeof(closed));

	Cell cells[getSize().x][getSize().y];
	cells[start.x][start.y] = Cell(start, 0,0,0);

	std::deque<Cell> open;
	open.push_back(Cell(start, 0,0,0));

	while(open.size() > 0) {
		Cell p = open.front();
		open.pop_front();

		closed[p.parent.x][p.parent.y] = true;

		for(int i = 0; i < 4; i++) {
			Vector2i next = p.parent + NEIGHBORS[i];
			if(inBounds(next)) {
				if(next == target) {
					cells[next.x][next.y].parent = p.parent;
					out.push_back(next);
					while(cells[next.x][next.y].parent != start) {
						next = cells[next.x][next.y].parent;
						out.push_back(next);
					}
					out.push_back(start);
					if(simplify)
						return simplifyPath(out);
					return out;
				}

				if(!closed[next.x][next.y] && getTileI(next.x, next.y) == 0) {
					double g = cells[p.parent.x][p.parent.y].g + 1.0;
					double h = vectorLength(p.parent, target);
					double f = g + h;

					if(cells[next.x][next.y].f > f) {
						if(debug != NULL)
							debug->setTileI(next.x, next.y, h);
						open.push_back(Cell(next, f));
						cells[next.x][next.y] = Cell(p.parent, f,g,h);
					}
				}
			}
		}
	}
	return out;
}

//Reduce path down to required corners
std::vector<Vector2i> Indexer::simplifyPath(std::vector<Vector2i> path) {
	if(path.size()<2)
		return path;

	std::vector<Vector2i> out;
	out.push_back(path[0]);

	sint start = 0;
	sint furthest = 1;
	while(furthest < path.size()-1) {
		while(furthest < path.size()-1 && lineEmpty(path[start], path[furthest+1]))
			furthest++;
		start = furthest;
		furthest = start + 1;
		out.push_back(path[start]);
	}
	out.push_back(path[path.size()-1]);
	return out;
}

void Indexer::drawPath(std::vector<Vector2i> path, int value) {
	for(Vector2i p : path)
		setTileI(p.x, p.y, value);
}

bool Indexer::inBounds(Vector2f pos) {
	return pos.x >= 0 && pos.x < getSize().x*getScale().x &&
		pos.y >= 0 && pos.y < getSize().y*getScale().y;
}

bool Indexer::inBounds(int x, int y) {
	return x >= 0 && x < getSize().x && y >= 0 && y < getSize().y;
}

Vector2f Indexer::snapPosition(Vector2f position) {
	int x = position.x / getScale().x;
	int y = position.y / getScale().y;
	return Vector2f(x * getScale().x, y * getScale().y);
}

//Get size of grid
Vector2i Indexer::getSize() {
	return previous->getSize();
}

//Get indexer scale
Vector2i Indexer::getScale() {
	if(previous != NULL)
		return scale*previous->getScale();
	return scale;
}

//Get previous indexer in stack
Indexer *Indexer::getPrevious() {
	return previous;
}

int Indexer::getFallback() {
	return fallback;
}

void Indexer::printGrid() {
	for(int y = 0; y < getSize().y; y++) {
		for(int x = 0; x < getSize().x; x++)
			std::cout << (char)getTileI(x,y);
		std::cout << "\n";
	}
}

//Convert file to int[][]
GridMaker::GridMaker(std::string file, int fallback) : Indexer(NULL, fallback, Vector2i(1, 1)) {
	char *mapFile = IO::openFile(file);
	char *line = mapFile;

	//Get maximum file size
	while(line[0] != '\0') {
		int i = 0;
		while(line[i] != '\n' && line[i] != '\0')
			i++;

		if(i > width)
			width = i;
		++height;

		line += i;
		while(line[0] == '\n' || line[0] == '\r')
			line++;
	}
	IO::closeFile(mapFile);

	//Build array
	this->tiles = new int*[height];
	for(int i = 0; i < height; i++) {
		tiles[i] = new int[width];
		for(int j = 0; j < width; j++)
			tiles[i][j] = fallback;
	}
	reload(file);
}

//Create blank int[][]
GridMaker::GridMaker(int width, int height, int fallback) : Indexer(NULL, fallback, Vector2i(1, 1)) {
	this->width = width;
	this->height = height;

	//Build array
	this->tiles = new int*[height];
	for(int i = 0; i < height; i++) {
		tiles[i] = new int[width];
		for(int j = 0; j < width; j++)
			tiles[i][j] = fallback;
	}
}

GridMaker::GridMaker(Indexer *copy) : Indexer(NULL, copy->getFallback(), Vector2i(1, 1)) {
	this->width = copy->getSize().x;
	this->height = copy->getSize().y;

	//Build array
	this->tiles = new int*[height];
	for(int i = 0; i < height; i++) {
		tiles[i] = new int[width];
		for(int j = 0; j < width; j++)
			tiles[i][j] = copy->getTileI(j, i);
	}
}

GridMaker::~GridMaker() {
	for(int y = 0; y < height; y++)
		delete[] tiles[y];
	delete[] tiles;
}

void GridMaker::reload(std::string file, int offset, Rect<int> border) {
	if(file == "")
		return;

	if(border.width == 0 || border.left + border.width > width)
		border.width = width-border.left;
	if(border.height == 0 || border.top + border.height > height)
		border.height = height-border.top;

	//Set reading variables
	int i = border.top;
	char *mapFile = IO::openFile(file);
	char *line = mapFile;

	//Read file by line
	while(line[0] != '\0' && i < border.top + border.height) {
		//Copy string
		int j = 0;
		while(line[j] != '\0' && line[j] != '\n' &&
			line[j] != '\r' && j < border.width) {

			tiles[i][j+border.left] = line[j] + offset;
			++j;
		}
		i++;

		if(line[j] != '\0') {
			while(line[j] == '\n' || line[j] == '\r')
				j++;
		}
		line += j;
	}
	IO::closeFile(mapFile);
	updates++;
}

void GridMaker::reload(Indexer *copy) {
	for(int y = 0; y < height; y++)
		for(int x = 0; x < width; x++)
			tiles[y][x] = copy->getTileI(x,y);
	updates++;
}

void GridMaker::save(std::string file) {
	if(file == "")
		return;

	const int width = getSize().x+1;
	const int height = getSize().y;

	char text[height*width+height];

	//Loop through tiles
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			text[y*width+x] = tiles[y][x];
		}
		text[y*width+width-1] = '\n';
	}
	text[(height-1)*width+width-1] = '\0';

	IO::writeFile(file, text);
}

//Get tile value
int GridMaker::getTileI(int x, int y) {
	if(inBounds(x, y))
		return tiles[y][x];
	else
		return fallback;
}

//Set tile value
void GridMaker::setTileI(int x, int y, int value) {
	if(inBounds(x, y)) {
		tiles[y][x] = value;
		updates++;
	}
}

uint GridMaker::getUpdateCount() {
	return updates;
}

//Get size of grid
Vector2i GridMaker::getSize() {
	return Vector2i(width, height);
}

//Concat 2 maps
std::map<int, int> operator+(const std::map<int, int> &first, const std::map<int, int> &second) {
	std::map<int, int> third;
	third.insert(first.begin(), first.end());
	third.insert(second.begin(), second.end());
	return third;
}