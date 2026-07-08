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