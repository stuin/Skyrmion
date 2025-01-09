#include <iostream>

#include "GridMaker.h"

#define QuadMap std::vector<std::array<int,5>>
#define SquareMap std::vector<std::array<int,10>>

/*
 * Facilitates working with 2x2 and 3x3 squares of tiles
 */

//Helper functions for building Quad indexer maps
QuadMap operator+(const QuadMap &first, const QuadMap &second);
bool operator==(const std::array<int,5> &lhs, const std::array<int,5> &rhs);
std::ostream& operator<<(std::ostream& os, const std::array<int,5> quad);
QuadMap genQuadRotations(std::array<int,5> quads, int size);
QuadMap genQuadRotations(QuadMap quads, int size);

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
	QuadIndexer(Indexer *previous, QuadMap _quads, int fallback, sf::Vector2i scale=sf::Vector2i(1,1), int _seed=0)
		: Indexer(previous, fallback, scale) {

		for(std::array<int,5> _quad : _quads){
			quads.insert_or_assign({
				_quad[0], _quad[1], _quad[2], _quad[3]
			}, _quad[4]);
		}
	}

	int getTileI(int x, int y) {
		if(inBounds(x, y) && inBounds(x+1,y+1))
			return mapQuad(
				getPrevious()->getTileI(x, y), getPrevious()->getTileI(x+1, y),
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

//Helper functions for building 3x3 Square indexer maps
SquareMap operator+(const SquareMap &first, const SquareMap &second);
bool operator==(const std::array<int,10> &lhs, const std::array<int,10> &rhs);
std::ostream& operator<<(std::ostream& os, const std::array<int,10> square);
SquareMap genSquareRotations(std::array<int,10> quads, int size);
SquareMap genSquareRotations(SquareMap quads, int size);

void printUniqueSquares(Indexer *indexes);
SquareMap readSquareFile(std::string filename);