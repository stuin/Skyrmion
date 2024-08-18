#include "GridMaker.h"

/*
 * Created by Stuart Irwin on 4/16/2019.
 * Generates and stores main tilemap
 */

//Convert file to uint[][]
GridMaker::GridMaker(std::string file) {
	std::string line;
	std::ifstream mapFile(file);

	//Get maximum file size
	while(std::getline(mapFile, line)) {
		if(line.size() > width)
			width = line.size();
		++height;
	}
	mapFile.close();

	//Build array
	this->tiles = new uint*[height];
	for(unsigned int i = 0; i < height; i++) {
		tiles[i] = new uint[width];
		for(unsigned int j = 0; j < width; j++)
			tiles[i][j] = ' ';
	}
	reload(file);
}

//Create blank uint[][]
GridMaker::GridMaker(unsigned int width, unsigned int height) {
	this->width = width;
	this->height = height;

	//Build array
	this->tiles = new uint*[height];
	for(unsigned int i = 0; i < height; i++) {
		tiles[i] = new uint[width];
		for(unsigned int j = 0; j < width; j++)
			tiles[i][j] = ' ';
	}
}

GridMaker::~GridMaker() {
	for(unsigned int y = 0; y < height; y++)
		delete[] tiles[y];
	delete[] tiles;
}

void GridMaker::reload(std::string file, uint offset, sf::Rect<uint> border) {
	if(border.width == 0 || border.left + border.width > width)
		border.width = width-border.left;
	if(border.height == 0 || border.top + border.height > height)
		border.height = height-border.top;

	//Set reading variables
	unsigned int i = border.top;
	std::string line;
	std::ifstream mapFile(file);

	//Read file by line
	while(std::getline(mapFile, line) && i < border.top + border.height) {
		//Copy string
		unsigned int j = border.left;
		while(line[j-border.left] != '\0' && line[j-border.left] != '\n' && j < border.left + border.width) {
			tiles[i][j] = line[j-border.left] + offset;
			++j;
		}
		i++;
	}
	mapFile.close();
}

//Set tile value
void GridMaker::setTile(unsigned int x, unsigned int y, uint value) {
	if(inBounds(x, y))
		tiles[y][x] = value;
}

//Get tile value
uint GridMaker::getTile(unsigned int x, unsigned int y) {
	if(inBounds(x, y))
		return tiles[y][x];
	else return ' ';
}

//Set all tiles
void GridMaker::clearTiles(uint value) {
	for(unsigned int y = 0; y < height; y++)
		for(unsigned int x = 0; x < width; x++)
			tiles[y][x] = value;
}

//Get size of grid
sf::Vector2i GridMaker::getSize() const {
	return sf::Vector2i(width, height);
}

//Check cords vs grid size
bool GridMaker::inBounds(unsigned int x, unsigned int y) const {
	return x < width && y < height;
}

void GridMaker::printGrid() {
	for(unsigned int y = 0; y < height; y++) {
		for(unsigned int x = 0; x < width; x++)
			std::cout << (char)tiles[y][x];
		std::cout << "\n";
	}
}

//Get value of tile
int Indexer::getTile(uint c) {
	int rOffset = 0;
	if(random != NULL) {
		auto limit = random->find(c);
		if(limit != random->end() && limit->second > 1)
			rOffset = std::rand() / ((RAND_MAX + 1u) / limit->second);
	}

	auto tile = indexes.find(c);
	if(tile != indexes.end())
		return tile->second + rOffset;
	return fallback;
}

//Get tile uint from grid
int Indexer::getTile(sf::Vector2f position) {
	unsigned int x = position.x / scale.x;
	unsigned int y = position.y / scale.y;
	if(grid->inBounds(x, y))
		return getTile(grid->getTile(x, y));
	return getTile(' ');
}

//Set tile in grid
void Indexer::setTile(sf::Vector2f position, int value) {
	unsigned int x = position.x / scale.x;
	unsigned int y = position.y / scale.y;
	if(grid->inBounds(x, y))
		grid->setTile(x, y, value);
}

//Convert uint[][] to int[][]
int* Indexer::indexGrid() {
	const unsigned int width = grid->getSize().x;
	const unsigned int height = grid->getSize().y;
	int* indexes = new int[width * height];
	//Loop through tiles
	for(unsigned int y = 0; y < height; y++)
		for(unsigned int x = 0; x < width; x++) {
			//Get tile texture index
			indexes[x + y * width] = getTile(grid->getTile(x, y));
		}

	return indexes;
}

//Run function on every square in grid
void Indexer::mapGrid(std::function<void(uint, sf::Vector2f)> func) {
	const unsigned int width = grid->getSize().x;
	const unsigned int height = grid->getSize().y;
	//Loop through tiles
	for(unsigned int y = 0; y < height; y++)
		for(unsigned int x = 0; x < width; x++) {
			sf::Vector2f pos = sf::Vector2f(x * scale.x, y * scale.y);
			func(grid->getTile(x, y), pos);
		}
}

void Indexer::addRandomizer(std::map<uint, int> *_limits) {
	random = _limits;
}

bool Indexer::inBounds(sf::Vector2f position) {
	unsigned int x = position.x / scale.x;
	unsigned int y = position.y / scale.y;
	return grid->inBounds(x, y);
}

sf::Vector2f Indexer::snapPosition(sf::Vector2f position) {
	unsigned int x = position.x / scale.x;
	unsigned int y = position.y / scale.y;
	return sf::Vector2f(x * scale.x, y * scale.y);
}

//Get size of grid
sf::Vector2i Indexer::getSize() {
	return grid->getSize();
}

//Get indexer scale
sf::Vector2i Indexer::getScale() {
	return scale;
}