#include "GridMaker.h"

#include <noise/noise.h>

/*
 * Random noise tiling
 */

//Add a pure random value to each tile, with a range of 0 to limit index
class RandomIndexer : public Indexer {
private:
	Indexer *limits = NULL;
	int linearPosition = 0;

public:
	int seed = 0;
	int multiplier;

	RandomIndexer(Indexer *previous, std::map<int, int> _limits, int _multiplier=1, sf::Vector2i scale=sf::Vector2i(1,1))
		: Indexer(previous, previous->fallback, scale), limits(new MapIndexer(previous, _limits, 0)), multiplier(_multiplier) {

	}

	RandomIndexer(Indexer *previous, Indexer *_limits, int _multiplier=1, sf::Vector2i scale=sf::Vector2i(1,1))
		: Indexer(previous, previous->fallback, scale), limits(_limits), multiplier(_multiplier) {

	}

	//Function copied from https://libnoise.sourceforge.net/noisegen/index.html
	//Modified for range 0.0 - 1.0
	double IntegerNoise(int n) {
		n = (n >> 13) ^ n;
		int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
		return ((double)nn / 1073741824.0) / 2.0;
	}

	//Consistant locational randomness
	int getTileI(int x, int y) {
		if(inBounds(x, y)) {
			int rOffset = 0;
			int limit = limits->getTileI(x, y);
			double input = IntegerNoise(x + y*getSize().x + seed*getSize().y*getSize().x);
			rOffset = (int)floor(input * limit);
			return getPrevious()->getTileI(x, y) + rOffset * multiplier;
		}
		return fallback;
	}

	//Backup linear random function
	int mapTile(int c) {
		int rOffset = 0;
		int limit = limits->mapTile(c);
		double input = IntegerNoise(linearPosition++);
		rOffset = (int)floor(input * limit);
		return getPrevious()->mapTile(c) + rOffset * multiplier;
	}
};

class NoiseIndexer : public Indexer {
private:
	Indexer *limits = NULL;
	noise::module::Module *noise;

public:
	int seed = 0;
	int multiplier;
	int linearPosition = 0;

	NoiseIndexer(Indexer *previous, std::map<int, int> _limits, noise::module::Module *_noise, int _multiplier=1, sf::Vector2i scale=sf::Vector2i(1,1))
		: Indexer(previous, previous->fallback, scale), limits(new MapIndexer(previous, _limits, 0)), noise(_noise), multiplier(_multiplier) {

	}

	NoiseIndexer(Indexer *previous, Indexer *_limits, noise::module::Module *_noise, int _multiplier=1, sf::Vector2i scale=sf::Vector2i(1,1))
		: Indexer(previous, previous->fallback, scale), limits(_limits), noise(_noise), multiplier(_multiplier) {

	}

	//Correct locational randomness
	int getTileI(int x, int y) {
		if(inBounds(x, y)) {
			int rOffset = 0;
			int limit = limits->getTileI(x, y);
			double input = noise->GetValue((double)x/getSize().x, (double)y/getSize().y, 0);
			rOffset = (int)floor(fmod(input+1, 1.0) * limit);
			return getPrevious()->getTileI(x, y) + rOffset * multiplier;
		}
		return fallback;
	}

	//Backup linear random function
	int mapTile(int c) {
		int rOffset = 0;
		int limit = limits->mapTile(c);
		double input = noise->GetValue(linearPosition++/100.0, 0, 0);
		rOffset = (int)floor(fmod(input+1, 1.0) * limit);
		return getPrevious()->mapTile(c) + rOffset * multiplier;
	}
};

//Constant value indexer
class ConstIndexer : public Indexer {
	sf::Vector2i size;

public:
	ConstIndexer(int fallback, int width, int height, int scaleX=1, int scaleY=1)
		: Indexer(NULL, fallback, sf::Vector2i(scaleX, scaleY)), size(width, height) {

	}

	ConstIndexer(int fallback, sf::Vector2i _size, sf::Vector2i scale)
		: Indexer(NULL, fallback, scale), size(_size) {

	}

	void setConst(int value) {
		fallback = value;
	}

	//Get tile value
	int getTileI(int x, int y) {
		return fallback;
	}

	//Do nothing
	void setTileI(int x, int y, int value) {

	}

	//Return actual size
	sf::Vector2i getSize() {
		return size;
	}

	//Get value of tile from map
	int mapTile(int c) override {
		return fallback;
	}
};