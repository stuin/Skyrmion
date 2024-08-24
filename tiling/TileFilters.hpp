#include "GridMaker.h"

class RandomIndexer : public Indexer {
private:
	const std::map<int, int> limits;
	int seed;

public:
	RandomIndexer(Indexer *previous, std::map<int, int> _limits, int fallback, sf::Vector2i scale=sf::Vector2i(1,1), int _seed=0)
		: Indexer(previous, fallback, scale), limits(_limits) {

	}

	int mapTile(int c) {
		int rOffset = 0;
		if(!limits.empty()) {
			auto limit = limits.find(c);
			if(limit != limits.end() && limit->second > 1)
				rOffset = std::rand() / ((RAND_MAX + 1u) / limit->second);
		}
		return c + rOffset;
	}
};