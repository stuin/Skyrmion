#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <string>

#include "../core/Vector.h"

/*
 * Generates and stores tiles for maps
 */

//Base class for reading tile properties, can be stacked
class Indexer {
private:
	Indexer *previous = NULL;
	const Vector2i scale;

public:
	int fallback;

	Indexer(Indexer *_previous, int _fallback, Vector2i _scale)
		: previous(_previous), scale(_scale), fallback(_fallback) {

	}

	virtual int mapTile(int c);

	//Indexing access functions
	int getTile(Vector2f position);
	void setTile(Vector2f position, int value);
	virtual int getTileI(int x, int y);
	virtual void setTileI(int x, int y, int value);
	bool getTileB(int x, int y, int place);
	void setTileB(int x, int y, int place, bool value);
	void mapGrid(std::function<void(int, Vector2f)> func);
	virtual uint getUpdateCount();
	void printGrid();

	//Check grid size
	virtual Vector2i getSize();
	bool inBounds(Vector2f position);
	bool inBounds(int x, int y);
	Vector2f snapPosition(Vector2f position);
	Vector2i getScale();
	Indexer *getPrevious();
};

//Lowest level indexer to store the actual grid
class GridMaker : public Indexer {
private:
	int height = 0;
	int width = 0;
	int **tiles;

	uint updates = 0;

public:
	//Build and convert grid
	GridMaker(std::string file, int fallback=' ');
	GridMaker(int width, int height, int fallback=' ');
	~GridMaker();
	void reload(std::string file, int offset=0, Rect<int> border=Rect<int>());
	void save(std::string file);

	//Set or get tiles
	int getTileI(int x, int y) override;
	void setTileI(int x, int y, int value) override;
	void clearTiles();
	uint getUpdateCount() override;

	//Check grid size
	Vector2i getSize() override;
};

//Common indexer to map from tile to property 1x1
class MapIndexer : public Indexer {
private:
	const std::map<int, int> indexes;
	bool keepOthers = false;

public:
	MapIndexer(Indexer *previous, std::map<int, int> _indexes, int fallback, int scaleX = 1, int scaleY = 1, bool _keepOthers=false)
		: Indexer(previous, fallback, Vector2i(scaleX, scaleY)), indexes(_indexes), keepOthers(_keepOthers) {

	}

	MapIndexer(Indexer *previous, std::map<int, int> _indexes, int fallback, Vector2i scale)
		: Indexer(previous, fallback, scale), indexes(_indexes) {

	}

	//Get value of tile from map
	int mapTile(int c) override {
		auto tile = indexes.find(c);
		if(tile != indexes.end())
			return tile->second;
		return keepOthers ? c : fallback;
	}
};

//Helper functions for building indexer maps
std::map<int, int> operator+(const std::map<int, int> &first, const std::map<int, int> &second);