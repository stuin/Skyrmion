#include "GridMaker.h"

/*
 * More specific Grid Indexer types
 */

//Add a random value to each tile, using a map to decide range 0-limit
class RandomIndexer : public Indexer {
private:
	const std::map<int, int> limits;
	int multiplier;
	int seed;

public:
	RandomIndexer(Indexer *previous, std::map<int, int> _limits, int fallback, int _multiplier=1, sf::Vector2i scale=sf::Vector2i(1,1), int _seed=0)
		: Indexer(previous, fallback, scale), limits(_limits), multiplier(_multiplier), seed(_seed) {

	}

	int mapTile(int c) {
		int rOffset = 0;
		if(!limits.empty()) {
			auto limit = limits.find(c);
			if(limit != limits.end() && limit->second > 1)
				rOffset = std::rand() / ((RAND_MAX + 1u) / limit->second);
		}
		return c + rOffset * multiplier;
	}
};