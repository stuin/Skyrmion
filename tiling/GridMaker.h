#pragma once

//SFML headers
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include <functional>
#include <fstream>
#include <iostream>
#include <string>

using uint = unsigned int;

/*
 * Created by Stuart Irwin on 4/15/2019.
 * Generates and stores tiles for maps
 */

class GridMaker {
private:
	uint height = 0;
	uint width = 0;
	uint **tiles;

public:
	//Build and convert grid
	GridMaker(std::string file);
	GridMaker(uint width, uint height);
	~GridMaker();
	void reload(std::string file, uint offset=0, sf::Rect<uint> border=sf::Rect<uint>());

	//Set or get tiles
	void setTile(uint x, uint y, uint value);
	uint getTile(uint x, uint y);
	void clearTiles(uint value);

	//Check grid size
	sf::Vector2i getSize() const;
	bool inBounds(unsigned int x, unsigned int y) const;

	void printGrid();
};

class Indexer {
private:
	const std::map<uint, int> indexes;
	const int fallback;
	const sf::Vector2i scale;
	GridMaker *grid;
	std::map<uint, int> random;

public:
	Indexer(GridMaker *new_grid, std::map<uint, int> new_indexes, int new_fallback,
		int scaleX = 1, int scaleY = 1)
		: indexes(new_indexes), fallback(new_fallback),
			scale(sf::Vector2i(scaleX, scaleY)), grid(new_grid) {

	}

	//Index use functions
	int getTile(uint c);
	int getTile(sf::Vector2f position);
	void setTile(sf::Vector2f position, int value);
	void mapGrid(std::function<void(uint, sf::Vector2f)> func);
	void addRandomizer(std::map<uint, int> limits);

	//Check grid size
	bool inBounds(sf::Vector2f position);
	sf::Vector2f snapPosition(sf::Vector2f position);
	sf::Vector2i getScale();
	sf::Vector2i getSize();
};