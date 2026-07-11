#pragma once

#include "../core/UpdateList.h"
#include "GridMaker.h"

//Constant value indexer
class ConstIndexer : public Indexer {
	Vector2i size;

public:
	ConstIndexer(int fallback, int width, int height, int scaleX=1, int scaleY=1)
		: Indexer(NULL, fallback, Vector2i(scaleX, scaleY)), size(width, height) {

	}

	ConstIndexer(int fallback, Vector2i _size, Vector2i scale)
		: Indexer(NULL, fallback, scale), size(_size) {

	}

	void setConst(int value) {
		fallback = value;
	}

	//Get tile value
	int getTileI(int x, int y) override {
		return fallback;
	}

	//Do nothing
	void setTileI(int x, int y, int value) override {

	}

	uint getUpdateCount() override {
		return 0;
	}

	//Return actual size
	Vector2i getSize() override {
		return size;
	}

	//Get value of tile from map
	int mapTile(int c) override {
		return fallback;
	}
};

//Linear math indexer
class LinearIndexer : public Indexer {
private:
	float multiplier = 1;
	int adder = 0;

public:
	LinearIndexer(Indexer *previous, float _mult, int _add, int fallback, int scaleX = 1, int scaleY = 1)
		: Indexer(previous, fallback, Vector2i(scaleX, scaleY)) {

		multiplier = _mult;
		adder = _add;
	}

	LinearIndexer(Indexer *previous, float _mult, int _add, int fallback, Vector2i scale)
		: Indexer(previous, fallback, scale) {

		multiplier = _mult;
		adder = _add;
	}

	//Get value of tile from map
	int mapTile(int c) override {
		return c * multiplier + adder;
	}
};

//Hexagon offset indexer
class HexIndexer : public Indexer {
public:
	HexIndexer(Indexer *previous, int fallback, int scaleX = 1, int scaleY = 1)
		: Indexer(previous, fallback, Vector2i(scaleX, scaleY)) {

	}

	HexIndexer(Indexer *previous, float _mult, int _add, int fallback, Vector2i scale)
		: Indexer(previous, fallback, scale) {

	}

	int getTile(Vector2f position) {
		if((int)(position.y / getScale().y) % 2 == 1)
			position.x += getScale().x/2;
		return getTileI(position.x / getScale().x, position.y / getScale().y);
	}

	void setTile(Vector2f position, int value) {
		if((int)(position.y / getScale().y) % 2 == 1)
			position.x += getScale().x/2;
		setTileI(position.x / getScale().x, position.y / getScale().y, value);
	}
};

//Value in filter
class FuncIndexer : public Indexer {
private:
	std::function<int(int)> func;

public:
	FuncIndexer(Indexer *previous, std::function<int(int)> _func, int fallback, int scaleX = 1, int scaleY = 1)
		: Indexer(previous, fallback, Vector2i(scaleX, scaleY)) {

		func = _func;
	}

	FuncIndexer(Indexer *previous, std::function<int(int)> _func, int fallback, Vector2i scale)
		: Indexer(previous, fallback, scale) {

		func = _func;
	}

	//Get value of tile from map
	int mapTile(int c) override {
		return func(c);
	}
};

class NodeIndexer : public Indexer {
private:
	int layer;

	std::function<int(Node*)> func;
	bool hasFunction = false;

public:
	NodeIndexer(Indexer *previous, int _layer, int scaleX = 1, int scaleY = 1)
		: Indexer(previous, 0, Vector2i(scaleX, scaleY)), layer(_layer) {

	}

	NodeIndexer(Indexer *previous, int _layer, Vector2i scale)
		: Indexer(previous, 0, scale), layer(_layer) {

	}

	Node *getNode(int id) {
		if(id == 0)
			return NULL;
		return UpdateList::getNode(layer, id);
	}

	Node *getNode(Vector2f position) {
		//std::cout << "Get pos " << position << " of id " << getTile(position) << "\n";
		return getNode(getTile(position));
	}

	void setNode(Vector2f position, Node *node) {
		int id = (node == NULL) ? 0 : node->getId();
		getPrevious()->setTileI(position.x, position.y, id);
		//std::cout << "Set pos " << position << " to id " << id << " " << getTile(position) << "\n";
	}

	//Get value of tile from map
	int mapTile(int c) override {
		if(!hasFunction)
			return c;
		Node *n = getNode(c);
		if(n != NULL)
			return func(n);
		return 0;
	}
};