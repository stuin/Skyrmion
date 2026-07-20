#include "GridMaker.h"

#include "../include/libnoise/src/noise/noise.h"

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

	RandomIndexer(Indexer *previous, std::map<int, int> _limits, int _multiplier=1, Vector2i scale=Vector2i(1,1))
		: Indexer(previous, previous->fallback, scale), limits(new MapIndexer(previous, _limits, 0)), multiplier(_multiplier) {

	}

	RandomIndexer(Indexer *previous, Indexer *_limits, int _multiplier=1, Vector2i scale=Vector2i(1,1))
		: Indexer(previous, previous->fallback, scale), limits(_limits), multiplier(_multiplier) {

	}

	//Function copied from https://libnoise.sourceforge.net/noisegen/index.html
	//Modified for range 0.0 - 1.0
	static double IntegerNoise(int n) {
		n = (n >> 13) ^ n;
		int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
		return ((double)nn / 1073741824.0) / 2.0;
	}

	//Consistant locational randomness
	int getTileI(int x, int y) override {
		if(inBounds(x, y)) {
			int rOffset = 0;
			int limit = limits->getTileI(x, y);
			double input = IntegerNoise(x + y*getSize().x + seed*getSize().y*getSize().x);
			rOffset = (int)floor(input * limit);
			rOffset = limitRange(rOffset, 0, limit);
			return getPrevious()->getTileI(x, y) + rOffset * multiplier;
		}
		return fallback;
	}

	//Backup linear random function
	int mapTile(int c) override {
		int rOffset = 0;
		int limit = limits->mapTile(c);
		double input = IntegerNoise(linearPosition++);
		rOffset = (int)floor(input * limit);
		rOffset = limitRange(rOffset, 0, limit);
		return getPrevious()->mapTile(c) + rOffset * multiplier;
	}
};

class NoiseIndexer : public Indexer {
private:
	Indexer *limits = NULL;
	noise::module::Module *noise;

public:
	int multiplier;
	int linearPosition = 0;
	uint noiseUpdateCount = 0;

	NoiseIndexer(Indexer *previous, std::map<int, int> _limits, noise::module::Module *_noise, int _multiplier=1, Vector2i scale=Vector2i(1,1))
		: Indexer(previous, previous->fallback, scale), limits(new MapIndexer(previous, _limits, 0)), noise(_noise), multiplier(_multiplier) {

	}

	NoiseIndexer(Indexer *previous, Indexer *_limits, noise::module::Module *_noise, int _multiplier=1, Vector2i scale=Vector2i(1,1))
		: Indexer(previous, previous->fallback, scale), limits(_limits), noise(_noise), multiplier(_multiplier) {

	}

	//Correct locational randomness
	int getTileI(int x, int y) override {
		if(inBounds(x, y)) {
			int rOffset = 0;
			int limit = limits->getTileI(x, y);
			double input = noise->GetValue((double)x/getSize().x, (double)y/getSize().y, 0);
			rOffset = (int)floor(fmod(input+1, 1.0) * limit);
			rOffset = limitRange(rOffset, 0, limit);
			return getPrevious()->getTileI(x, y) + rOffset * multiplier;
		}
		return fallback;
	}

	//Backup linear random function
	int mapTile(int c) override {
		int rOffset = 0;
		int limit = limits->mapTile(c);
		double input = noise->GetValue(linearPosition++/100.0, 0, 0);
		rOffset = (int)floor(fmod(input+1, 1.0) * limit);
		rOffset = limitRange(rOffset, 0, limit);
		return getPrevious()->mapTile(c) + rOffset * multiplier;
	}

	uint getUpdateCount() override {
		return getPrevious()->getUpdateCount() + noiseUpdateCount;
	}
};

class NoiseGrid : public Indexer {
private:
	noise::module::Module *noise;
	Vector2i size;

public:
	int limit;
	int multiplier;
	int linearPosition = 0;
	int noiseUpdateCount = 0;

	NoiseGrid(noise::module::Module *_noise, Vector2i _size, int _limit, int _multiplier=1, Vector2i scale=Vector2i(1,1))
		: Indexer(NULL, 0, scale), noise(_noise), size(_size), limit(_limit), multiplier(_multiplier) {

	}

	uint getUpdateCount() {
		return noiseUpdateCount;
	}

	//Get size of grid
	Vector2i getSize() {
		return size;
	}

	//Correct locational randomness
	int getTileI(int x, int y) override {
		if(inBounds(x, y)) {
			double input = noise->GetValue((double)x/getSize().x, (double)y/getSize().y, 0);
			int rOffset = (int)floor(fmod(input+1, 1.0) * limit);
			rOffset = limitRange(rOffset, 0, limit);
			return rOffset * multiplier;
		}
		return fallback;
	}

	//Backup linear random function
	int mapTile(int c) override {
		double input = noise->GetValue(linearPosition++/100.0, 0, 0);
		int rOffset = (int)floor(fmod(input+1, 1.0) * limit);
		rOffset = limitRange(rOffset, 0, limit);
		return rOffset * multiplier;
	}

	void save(std::string file, char min='0') {
		if(file == "")
			return;

		const int width = getSize().x+1;
		const int height = getSize().y;
		std::cout << getSize() << "\n";

		char text[height*width+height];

		//Loop through tiles
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				text[y*width+x] = getTileI(x, y)+min;
			}
			text[y*width+width-1] = '\n';
		}
		text[(height-1)*width+width-1] = '\0';

		IO::writeFile(file, text);
	}
};