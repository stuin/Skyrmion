#pragma once

//SFML headers
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include <functional>
#include <fstream>
#include <iostream>
#include <string>

/*
 * Created by Stuart Irwin on 4/15/2019.
 * Generates and stores tiles for maps
 */

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

	//Check grid size
	virtual sf::Vector2i getSize();
	bool inBounds(sf::Vector2f position);
	bool inBounds(int x, int y);
	sf::Vector2f snapPosition(sf::Vector2f position);
	sf::Vector2i getScale();
};

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
	void printGrid();
};

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