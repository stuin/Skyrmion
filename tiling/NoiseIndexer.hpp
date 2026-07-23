#include "GridMaker.h"

//#include "../include/libnoise/src/noise/noise.h"
#include "../include/FastNoiseLite.h"
//#include "../include/PerlinNoise.hpp"

#define NOISE_FOREACH(E) \
    E(NOISEOpenSimplex2) \
	E(NOISEOpenSimplex2S) \
	E(NOISECellular) \
	E(NOISEPerlin) \
	E(NOISEValueCubic) \
	E(NOISEValue) \
	E(NOISEMAX) \

NAMED_ENUM(NOISE);

static const FastNoiseLite::NoiseType NOISE_TYPES[] = {
	FastNoiseLite::NoiseType_OpenSimplex2,
	FastNoiseLite::NoiseType_OpenSimplex2S,
	FastNoiseLite::NoiseType_Cellular,
	FastNoiseLite::NoiseType_Perlin,
	FastNoiseLite::NoiseType_ValueCubic,
	FastNoiseLite::NoiseType_Value,
};

/*
 * Random noise tiling
 */

//Add a pure random value to each tile, with a range of 0 to limit index
class RandomIndexer : public Indexer {
private:
	Indexer *limits = NULL;
	int linearPosition = 0;

	Vector2i size;
	int rawLimit = 0;

	sint seed = 0;

public:
	int noiseUpdateCount = 0;
	int multiplier;

	RandomIndexer(Indexer *previous, std::map<int, int> _limits, sint _seed, int _multiplier=1, Vector2i scale=Vector2i(1,1))
		: Indexer(previous, previous->fallback, scale), limits(new MapIndexer(previous, _limits, 0)), seed(_seed), multiplier(_multiplier) {

	}

	RandomIndexer(Indexer *previous, Indexer *_limits, sint _seed, int _multiplier=1, Vector2i scale=Vector2i(1,1))
		: Indexer(previous, previous->fallback, scale), limits(_limits), seed(_seed), multiplier(_multiplier) {

	}

	RandomIndexer(Vector2i _size, int _limit, sint _seed, int _multiplier=1, Vector2i scale=Vector2i(1,1))
		: Indexer(NULL, 0, scale), size(_size), rawLimit(_limit), seed(_seed), multiplier(_multiplier) {

	}

	void setSeed(sint _seed) {
		seed = _seed;
		noiseUpdateCount++;
	}

	sint getSeed() {
		return seed;
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
			int limit = rawLimit;
			int previous = 0;
			if(limits != NULL) {
				limit = limits->getTileI(x, y);
				previous = getPrevious()->getTileI(x, y);
			}

			double input = IntegerNoise(x + y*getSize().x + seed*getSize().y*getSize().x);
			int rOffset = (int)floor(input * limit);
			rOffset = limitRange(rOffset, 0, limit);
			return previous + rOffset * multiplier;
		}
		return fallback;
	}

	//Backup linear random function
	int mapTile(int c) override {
		int limit = rawLimit;
		int previous = 0;
		if(limits != NULL) {
			limit = limits->mapTile(c);
			previous = getPrevious()->mapTile(c);
		}

		double input = IntegerNoise(linearPosition++ + seed);
		int rOffset = (int)floor(input * limit);
		rOffset = limitRange(rOffset, 0, limit);
		return previous + rOffset * multiplier;
	}

	uint getUpdateCount() override {
		if(getPrevious() == NULL)
			return noiseUpdateCount;
		return getPrevious()->getUpdateCount() + noiseUpdateCount + limits->getUpdateCount();
	}

	//Get size of grid
	Vector2i getSize() {
		if(getPrevious() == NULL)
			return size;
		return getPrevious()->getSize();
	}
};

class NoiseIndexer : public Indexer {
private:
	Indexer *limits = NULL;

	Vector2i size;
	int rawLimit = 0;

	sint seed;
	FastNoiseLite noise;
	int noiseType;

	float frequency = 1;
	float persistence = 0.5;
	int octaves = 1;

public:
	int multiplier;
	int linearPosition = 0;
	uint noiseUpdateCount = 0;

	NoiseIndexer(Indexer *previous, std::map<int, int> _limits, sint _seed, int nType, int _multiplier=1, Vector2i scale=Vector2i(1,1))
		: Indexer(previous, previous->fallback, scale), limits(new MapIndexer(previous, _limits, 0)), seed(_seed), multiplier(_multiplier) {

		noise.SetNoiseType(NOISE_TYPES[nType]);
		noise.SetSeed(seed);
		noise.SetFrequency(1);
		noiseType = nType;
	}

	NoiseIndexer(Indexer *previous, Indexer *_limits, sint _seed, int nType, int _multiplier=1, Vector2i scale=Vector2i(1,1))
		: Indexer(previous, previous->fallback, scale), limits(_limits), seed(_seed), multiplier(_multiplier) {

		noise.SetNoiseType(NOISE_TYPES[nType]);
		noise.SetSeed(seed);
		noise.SetFrequency(1);
		noiseType = nType;
	}

	NoiseIndexer(Vector2i _size, int _limit, sint _seed, int nType, int _multiplier=1, Vector2i scale=Vector2i(1,1))
		: Indexer(NULL, 0, scale), size(_size), rawLimit(_limit), seed(_seed), multiplier(_multiplier) {

		noise.SetNoiseType(NOISE_TYPES[nType]);
		noise.SetSeed(seed);
		noise.SetFrequency(1);
		noiseType = nType;
	}

	void setSeed(sint _seed) {
		seed = _seed;
		noise.SetSeed(_seed);
		noiseUpdateCount++;
	}

	void setFrequency(float _frequency) {
		noise.SetFrequency(_frequency);
		frequency = _frequency;
		noiseUpdateCount++;
	}

	void setOctaves(int _octaves, float _persistence) {
		if(_octaves > 1)
			noise.SetFractalType(FastNoiseLite::FractalType_FBm);
		else
			noise.SetFractalType(FastNoiseLite::FractalType_None);

		noise.SetFractalOctaves(_octaves);
		noise.SetFractalGain(_persistence);

		octaves = _octaves;
		persistence = _persistence;
		noiseUpdateCount++;
	}

	void setNoiseType(int nType) {
		noise.SetNoiseType(NOISE_TYPES[nType]);
		noiseType = nType;
		noiseUpdateCount++;
	}

	sint getSeed() {
		return seed;
	}
	float getFrequency() {
		return frequency;
	}
	float getPersistence() {
		return persistence;
	}
	int getOctaveCount() {
		return octaves;
	}
	int getNoiseType() {
		return noiseType;
	}

	//Correct locational randomness
	int getTileI(int x, int y) override {
		if(inBounds(x, y)) {
			int limit = rawLimit;
			int previous = 0;
			if(limits != NULL) {
				limit = limits->getTileI(x, y);
				previous = getPrevious()->getTileI(x, y);
			}

			//double input = noise.octave2D_11((double)x/getSize().x*frequency, (double)y/getSize().y*frequency, octaves, persistence);
			float input = noise.GetNoise((float)x/getSize().x, (float)y/getSize().y);
			int rOffset = (int)floor(fmod(input+1, 1.0) * limit);
			rOffset = limitRange(rOffset, 0, limit);

			return previous + rOffset * multiplier;
		}
		return fallback;
	}

	//Backup linear random function
	int mapTile(int c) override {
		int limit = rawLimit;
		int previous = 0;
		if(limits != NULL) {
			limit = limits->mapTile(c);
			previous = getPrevious()->mapTile(c);
		}

		//double input = noise.octave1D_11(linearPosition++/100.0*frequency, octaves, persistence);
		float input = noise.GetNoise(linearPosition++/100.0, 0.0);
		int rOffset = (int)floor(fmod(input+1, 1.0) * limit);
		rOffset = limitRange(rOffset, 0, limit);

		return previous + rOffset * multiplier;
	}

	uint getUpdateCount() override {
		if(getPrevious() == NULL)
			return noiseUpdateCount;
		return getPrevious()->getUpdateCount() + noiseUpdateCount + limits->getUpdateCount();
	}

	//Get size of grid
	Vector2i getSize() {
		if(getPrevious() == NULL)
			return size;
		return getPrevious()->getSize();
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