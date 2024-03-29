#pragma once

//SFML headers
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include <fstream>
#include <string>
#include <functional>

/*
 * Created by Stuart Irwin on 4/15/2019.
 * Generates and stores tiles for maps
 */

class GridMaker {
public:
	//Build and convert grid
	GridMaker(std::string file);
	GridMaker(unsigned int width, unsigned int height);
	~GridMaker();
	void reload(std::string file);

	//Set or get tiles
	void setTile(unsigned int x, unsigned int y, char value);
	char getTile(unsigned int x, unsigned int y);
	void clearTiles(char value);

	//Check grid size
	sf::Vector2i getSize() const;
	bool inBounds(unsigned int x, unsigned int y) const;

private:
	unsigned int height = 0;
	unsigned int width = 0;
	char **tiles;
};

class Indexer {
private:
	GridMaker *grid;
	const std::map<char, int> indexes;
	const int fallback;
	const sf::Vector2i scale;

public:
	Indexer(GridMaker *new_grid, std::map<char, int> new_indexes, int new_fallback,
		int scaleX = 1, int scaleY = 1)
		: grid(new_grid), indexes(new_indexes), fallback(new_fallback),
			scale(sf::Vector2i(scaleX, scaleY)) {

	}

	//Index use functions
	int getTile(char c);
	int getTile(sf::Vector2f position);
	void setTile(sf::Vector2f position, int value);
	int* indexGrid();
	void mapGrid(std::function<void(char, sf::Vector2f)> func);

	//Check grid size
	bool inBounds(sf::Vector2f position);
	sf::Vector2i getScale();
	sf::Vector2i getSize();
};