#include "GridMaker.h"

/*
 * Generates and stores main tilemap
 */

int Indexer::mapTile(int c) {
	return c;
}

//Scale vector before retrieving value
int Indexer::getTile(sf::Vector2f position) {
	return getTileI(position.x / getScale().x, position.y / getScale().y);
}

//Get tile int from previous
int Indexer::getTileI(int x, int y) {
	if(inBounds(x, y))
		return mapTile(previous->getTileI(x, y));
	return fallback;
}

//Scale and set tile
void Indexer::setTile(sf::Vector2f position, int value) {
	setTileI(position.x / getScale().x, position.y / getScale().y, value);
}

//Set tile in grid
void Indexer::setTileI(int x, int y, int value) {
	if(inBounds(x, y))
		previous->setTileI(x, y, value);
}

//Run function on every square in grid
void Indexer::mapGrid(std::function<void(int, sf::Vector2f)> func) {
	const int width = getSize().x;
	const int height = getSize().y;
	//Loop through tiles
	for(int y = 0; y < height; y++)
		for(int x = 0; x < width; x++) {
			sf::Vector2f pos = sf::Vector2f(x * getScale().x, y * getScale().y);
			func(getTileI(x, y), pos);
		}
}

bool Indexer::inBounds(sf::Vector2f pos) {
	return pos.x >= 0 && pos.x < getSize().x*getScale().x &&
		pos.y >= 0 && pos.y < getSize().y*getScale().y;
}

bool Indexer::inBounds(int x, int y) {
	return x >= 0 && x < getSize().x && y >= 0 && y < getSize().y;
}

sf::Vector2f Indexer::snapPosition(sf::Vector2f position) {
	int x = position.x / getScale().x;
	int y = position.y / getScale().y;
	return sf::Vector2f(x * getScale().x, y * getScale().y);
}

//Get size of grid
sf::Vector2i Indexer::getSize() {
	return previous->getSize();
}

//Get indexer scale
sf::Vector2i Indexer::getScale() {
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
GridMaker::GridMaker(std::string file, int fallback) : Indexer(NULL, fallback, sf::Vector2i(1, 1)) {
	std::string line;
	std::ifstream mapFile(file);

	//Get maximum file size
	while(std::getline(mapFile, line)) {
		if((int)line.size() > width)
			width = line.size();
		++height;
	}
	mapFile.close();

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
GridMaker::GridMaker(int width, int height, int fallback) : Indexer(NULL, fallback, sf::Vector2i(1, 1)) {
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

void GridMaker::reload(std::string file, int offset, sf::Rect<int> border) {
	if(border.width == 0 || border.left + border.width > width)
		border.width = width-border.left;
	if(border.height == 0 || border.top + border.height > height)
		border.height = height-border.top;

	//Set reading variables
	int i = border.top;
	std::string line;
	std::ifstream mapFile(file);

	//Read file by line
	while(std::getline(mapFile, line) && i < border.top + border.height) {
		//Copy string
		int j = border.left;
		while(line[j-border.left] != '\0' && line[j-border.left] != '\n' &&
			line[j-border.left] != '\r' && j < border.left + border.width) {

			tiles[i][j] = line[j-border.left] + offset;
			++j;
		}
		i++;
	}
	mapFile.close();
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
	if(inBounds(x, y))
		tiles[y][x] = value;
}

//Set all tiles
void GridMaker::clearTiles() {
	for(int y = 0; y < height; y++)
		for(int x = 0; x < width; x++)
			tiles[y][x] = fallback;
}

//Get size of grid
sf::Vector2i GridMaker::getSize() {
	return sf::Vector2i(width, height);
}

//Concat 2 maps
std::map<int, int> operator+(const std::map<int, int> &first, const std::map<int, int> &second) {
	std::map<int, int> third;
	third.insert(first.begin(), first.end());
	third.insert(second.begin(), second.end());
	return third;
}

//Concat 2 quad maps
QuadMap operator+(const QuadMap &first, const QuadMap &second) {
	QuadMap third;
	third.insert(third.begin(), first.begin(), first.end());
	third.insert(third.begin(), second.begin(), second.end());
	return third;
}

bool operator==(const std::array<int,5> &lhs, const std::array<int,5> &rhs) {
	return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
}

std::ostream& operator<<(std::ostream& os, const std::array<int,5> quad) {
	return os << (char)quad[0] << (char)quad[1] << "\n" <<
		(char)quad[2] << (char)quad[3] << "\n";
}

/*
0 1
2 3
*/

//Generate all rotations for a quad of tiles
QuadMap genQuadRotations(std::array<int,5> quad, int size) {
	QuadMap out;
	out.push_back(quad);
	//std::cout << out.back() << "\n";

	if(quad[0] != quad[1] || quad[1] != quad[2] || quad[2] != quad[3]) {
		out.push_back({quad[2], quad[0], quad[3], quad[1], quad[4]+size});
		out.push_back({quad[3], quad[2], quad[1], quad[0], quad[4]+size*2});
		out.push_back({quad[1], quad[3], quad[0], quad[2], quad[4]+size*3});

		//Check if horizontal flip = same tile arrangement
		std::array<int,5> flipQuad = {quad[1], quad[0], quad[3], quad[2], quad[4]+size*4};
		if(std::find(out.begin(), out.end(), flipQuad) == out.end()) {
			out.push_back(flipQuad);
			out.push_back({flipQuad[2], flipQuad[0], flipQuad[3], flipQuad[1], flipQuad[4]+size});
			out.push_back({flipQuad[3], flipQuad[2], flipQuad[1], flipQuad[0], flipQuad[4]+size*2});
			out.push_back({flipQuad[1], flipQuad[3], flipQuad[0], flipQuad[2], flipQuad[4]+size*3});
		}
	}
	return out;
}

//Generate tile rotations for each quad in map
QuadMap genQuadRotations(QuadMap quads, int size) {
	QuadMap out;
	for(std::array<int,5> baseQuad : quads) {
		for(std::array<int,5> quad : genQuadRotations(baseQuad, size))
			if(std::find(out.begin(), out.end(), quad) == out.end())
				out.push_back(quad);
	}
	return out;
}

//Concat 2 square maps
SquareMap operator+(const SquareMap &first, const SquareMap &second) {
	SquareMap third;
	third.insert(third.begin(), first.begin(), first.end());
	third.insert(third.begin(), second.begin(), second.end());
	return third;
}

bool operator==(const std::array<int,10> &lhs, const std::array<int,10> &rhs) {
	for(int i = 0; i < 9; i++)
		if(lhs[i] != rhs[i])
			return false;
	return true;
}

std::ostream& operator<<(std::ostream& os, const std::array<int,10> square) {
	return os <<
		(char)square[0] << (char)square[1] << (char)square[2] << "\n" <<
		(char)square[3] << (char)square[4] << (char)square[5] << "\n" <<
		(char)square[6] << (char)square[7] << (char)square[8] << "\n";
}

/*
0 1 2
3 4 5
6 7 8
*/

//Generate all rotations for a 3x3 square of tiles
SquareMap genSquareRotations(std::array<int,10> square, int size) {
	SquareMap out;
	out.push_back(square);
	//std::cout << out.back() << "\n";

	out.push_back({
		square[6], square[3], square[0],
		square[7], square[4], square[1],
		square[8], square[5], square[2], square[9]+size
	});
	out.push_back({
		square[8], square[7], square[6],
		square[5], square[4], square[3],
		square[2], square[1], square[0], square[9]+size*2
	});
	//out.push_back({square[3], square[2], square[1], square[0], square[4]+size*2});
	out.push_back({
		square[2], square[5], square[8],
		square[1], square[4], square[7],
		square[0], square[3], square[6], square[9]+size*3
	});
	//out.push_back({square[1], square[3], square[0], square[2], square[4]+size*3});

	//Check if horizontal flip = same tile arrangement
	std::array<int,10> flipSquare = {
		square[2], square[1], square[0],
		square[5], square[4], square[3],
		square[8], square[7], square[6], square[9]+size*4
	};
	if(std::find(out.begin(), out.end(), flipSquare) == out.end()) {
		out.push_back(flipSquare);
		out.push_back({
			flipSquare[6], flipSquare[3], flipSquare[0],
			flipSquare[7], flipSquare[4], flipSquare[1],
			flipSquare[8], flipSquare[5], flipSquare[2], flipSquare[9]+size
		});
		out.push_back({
			flipSquare[8], flipSquare[7], flipSquare[6],
			flipSquare[5], flipSquare[4], flipSquare[3],
			flipSquare[2], flipSquare[1], flipSquare[0], flipSquare[9]+size*2
		});
		out.push_back({
			flipSquare[2], flipSquare[5], flipSquare[8],
			flipSquare[1], flipSquare[4], flipSquare[7],
			flipSquare[0], flipSquare[3], flipSquare[6], flipSquare[9]+size*3
		});
	}
	return out;
}

//Generate tile rotations for each 3x3 square in map
SquareMap genSquareRotations(SquareMap squares, int size) {
	SquareMap out;
	for(std::array<int,10> baseSquare : squares) {
		for(std::array<int,10> square : genSquareRotations(baseSquare, size))
			if(std::find(out.begin(), out.end(), square) == out.end())
				out.push_back(square);
	}
	return out;
}

void printUniqueSquares(Indexer *indexes) {
	SquareMap out;
	std::cout << "Finding squares\n";
	for(int y = 1; y < indexes->getSize().y-1; y++) {
		for(int x = 1; x < indexes->getSize().x-1; x++) {
			std::array<int,10> baseSquare = {
				indexes->getTileI(x-1,y-1), indexes->getTileI(x,y-1), indexes->getTileI(x+1,y-1),
				indexes->getTileI(x-1,y),   indexes->getTileI(x,y),   indexes->getTileI(x+1,y),
				indexes->getTileI(x-1,y+1), indexes->getTileI(x,y+1), indexes->getTileI(x+1,y+1), 0
			};

			bool found = false;
			for(std::array<int,10> square : genSquareRotations(baseSquare, 0))
				if(!found && std::find(out.begin(), out.end(), square) != out.end())
					found = true;
			if(!found) {
				out.push_back(baseSquare);
				std::cout << baseSquare << "\n";
			}
		}
	}
}