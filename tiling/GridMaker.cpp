#include "GridMaker.h"

#include "../core/UpdateList.h"

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

uint Indexer::getUpdateCount() {
	return previous->getUpdateCount();
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
	return scale;
}

//Get previous indexer in stack
Indexer *Indexer::getPrevious() {
	return previous;
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
	char *mapFile = UpdateList::openFile(file);
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
	UpdateList::closeFile(mapFile);

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

GridMaker::~GridMaker() {
	for(int y = 0; y < height; y++)
		delete[] tiles[y];
	delete[] tiles;
}

void GridMaker::reload(std::string file, int offset, Rect<int> border) {
	if(border.width == 0 || border.left + border.width > width)
		border.width = width-border.left;
	if(border.height == 0 || border.top + border.height > height)
		border.height = height-border.top;

	//Set reading variables
	int i = border.top;
	char *mapFile = UpdateList::openFile(file);
	char *line = mapFile;

	//Read file by line
	while(line[0] != '\0' && i < border.top + border.height) {
		//Copy string
		int j = border.left;
		while(line[j-border.left] != '\0' && line[j-border.left] != '\n' &&
			line[j-border.left] != '\r' && j < border.left + border.width) {

			tiles[i][j] = line[j-border.left] + offset;
			++j;
		}
		i++;

		line += j;
		while(line[0] == '\n' || line[0] == '\r')
			line++;
	}
	UpdateList::closeFile(mapFile);
	updates++;
}

void GridMaker::save(std::string file) {
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

	UpdateList::writeFile(file, text);
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

//Set all tiles
void GridMaker::clearTiles() {
	for(int y = 0; y < height; y++)
		for(int x = 0; x < width; x++)
			tiles[y][x] = fallback;
	updates++;
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