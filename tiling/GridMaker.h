#pragma once

//SFML headers
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include <functional>
#include <fstream>
#include <iostream>
#include <string>

#define QuadMap std::vector<std::array<int,5>>
#define SquareMap std::vector<std::array<int,10>>

/*
 * Generates and stores tiles for maps
 */

//Base class for reading tile properties, can be stacked
class Indexer {
private:
	Indexer *previous = NULL;
	const sf::Vector2i scale;

public:
	const int fallback;

	Indexer(Indexer *_previous, int _fallback, sf::Vector2i _scale)
		: previous(_previous), scale(_scale), fallback(_fallback) {

	}

	virtual int mapTile(int c);

	//Indexing access functions
	int getTile(sf::Vector2f position);
	void setTile(sf::Vector2f position, int value);
	virtual int getTileI(int x, int y);
	virtual void setTileI(int x, int y, int value);
	void mapGrid(std::function<void(int, sf::Vector2f)> func);
	void printGrid();

	//Check grid size
	virtual sf::Vector2i getSize();
	bool inBounds(sf::Vector2f position);
	bool inBounds(int x, int y);
	sf::Vector2f snapPosition(sf::Vector2f position);
	sf::Vector2i getScale();
	Indexer *getPrevious();
};

//Lowest level indexer to store the actual grid
class GridMaker : public Indexer {
private:
	int height = 0;
	int width = 0;
	int **tiles;

public:
	//Build and convert grid
	GridMaker(std::string file, int fallback=' ');
	GridMaker(int width, int height, int fallback=' ');
	~GridMaker();
	void reload(std::string file, int offset=0, sf::Rect<int> border=sf::Rect<int>());

	//Set or get tiles
	int getTileI(int x, int y) override;
	void setTileI(int x, int y, int value) override;
	void clearTiles();

	//Check grid size
	sf::Vector2i getSize() override;
};

//Common indexer to map from tile to property 1x1
class MapIndexer : public Indexer {
private:
	const std::map<int, int> indexes;

public:
	MapIndexer(Indexer *previous, std::map<int, int> _indexes, int fallback, int scaleX = 1, int scaleY = 1)
		: Indexer(previous, fallback, sf::Vector2i(scaleX, scaleY)), indexes(_indexes) {

	}

	MapIndexer(Indexer *previous, std::map<int, int> _indexes, int fallback, sf::Vector2i scale)
		: Indexer(previous, fallback, scale), indexes(_indexes) {

	}

	//Get value of tile from map
	int mapTile(int c) override {
		auto tile = indexes.find(c);
		if(tile != indexes.end())
			return tile->second;
		return fallback;
	}
};

//Helper functions for building indexer maps
std::map<int, int> operator+(const std::map<int, int> &first, const std::map<int, int> &second);

//Helper functions for building Quad indexer maps
QuadMap operator+(const QuadMap &first, const QuadMap &second);
bool operator==(const std::array<int,5> &lhs, const std::array<int,5> &rhs);
std::ostream& operator<<(std::ostream& os, const std::array<int,5> quad);
QuadMap genQuadRotations(std::array<int,5> quads, int size);
QuadMap genQuadRotations(QuadMap quads, int size);

//Helper functions for building 3x3 Square indexer maps
SquareMap operator+(const SquareMap &first, const SquareMap &second);
bool operator==(const std::array<int,10> &lhs, const std::array<int,10> &rhs);
std::ostream& operator<<(std::ostream& os, const std::array<int,10> square);
SquareMap genSquareRotations(std::array<int,10> quads, int size);
SquareMap genSquareRotations(SquareMap quads, int size);

void printUniqueSquares(Indexer *indexes);