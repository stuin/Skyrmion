#include "GridMaker.h"

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

struct QuadCmp {
	bool operator()(const std::array<int,4> &lhs, const std::array<int,4> &rhs) const {
		return lhs[0] < rhs[0] || lhs[1] < rhs[1] || lhs[2] < rhs[2] || lhs[3] < rhs[3];
	}
};

struct QuadHash {
	std::size_t operator()(const std::array<int,4> &k) const {
		return std::hash<int>()(k[0]) + std::hash<int>()(k[1]) * 31 +
		std::hash<int>()(k[2]) * 37 + std::hash<int>()(k[3]) * 41;
	}
};

struct QuadEqual {
	bool operator()(const std::array<int,4> &lhs, const std::array<int,4> &rhs) const {
		return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
	}
};

//Map 4 tiles into 1 in a square
class QuadIndexer : public Indexer {
private:
	//std::map<std::array<int,4>, int, QuadCmp> quads;
	std::unordered_map<std::array<int,4>, int, QuadHash, QuadEqual> quads;

public:
	QuadIndexer(Indexer *previous, std::vector<std::array<int,5>> _quads, int fallback, sf::Vector2i scale=sf::Vector2i(1,1), int _seed=0)
		: Indexer(previous, fallback, scale) {

		for(std::array<int,5> _quad : _quads){
			quads.insert_or_assign({
				_quad[0], _quad[1], _quad[2], _quad[3]
			}, _quad[4]);
		}
	}

	int getTileI(int x, int y) {
		if(inBounds(x, y) && inBounds(x+1,y+1))
			return mapQuad(getPrevious()->getTileI(x, y), getPrevious()->getTileI(x+1, y),
				getPrevious()->getTileI(x, y+1), getPrevious()->getTileI(x+1, y+1));
		return fallback;
	}

	int mapQuad(int ul, int ur, int bl, int br) {
		auto tile = quads.find({ul, ur, bl, br});
		if(tile != quads.end())
			return tile->second;
		//std::cout << (char)ul << (char)ur << "\n" << (char)bl << (char)br << "\n\n";
		return fallback;
	}
};