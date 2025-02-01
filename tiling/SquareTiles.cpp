#include "SquareTiles.h"

//Concat 2 quad maps
QuadMap operator+(const QuadMap &first, const QuadMap &second) {
	QuadMap third;
	third.insert(third.end(), first.begin(), first.end());
	third.insert(third.end(), second.begin(), second.end());
	return third;
}

bool operator==(const std::array<int,5> &lhs, const std::array<int,5> &rhs) {
	return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
}

std::ostream& operator<<(std::ostream& os, const std::array<int,5> quad) {
	return os << (char)quad[0] << (char)quad[1] << "\n" <<
		(char)quad[2] << (char)quad[3] << "\n";
}

/*
0 1
2 3
*/

//Generate all rotations for a quad of tiles
QuadMap genQuadRotations(std::array<int,5> quad, int size) {
	QuadMap out;
	out.push_back(quad);
	//std::cout << out.back() << "\n";

	if(quad[0] != quad[1] || quad[1] != quad[2] || quad[2] != quad[3]) {
		out.push_back({quad[2], quad[0], quad[3], quad[1], quad[4]+size});
		out.push_back({quad[3], quad[2], quad[1], quad[0], quad[4]+size*2});
		out.push_back({quad[1], quad[3], quad[0], quad[2], quad[4]+size*3});

		//Check if horizontal flip = same tile arrangement
		std::array<int,5> flipQuad = {quad[1], quad[0], quad[3], quad[2], quad[4]+size*4};
		if(std::find(out.begin(), out.end(), flipQuad) == out.end()) {
			out.push_back(flipQuad);
			out.push_back({flipQuad[2], flipQuad[0], flipQuad[3], flipQuad[1], flipQuad[4]+size});
			out.push_back({flipQuad[3], flipQuad[2], flipQuad[1], flipQuad[0], flipQuad[4]+size*2});
			out.push_back({flipQuad[1], flipQuad[3], flipQuad[0], flipQuad[2], flipQuad[4]+size*3});
		}
	}
	return out;
}

//Generate tile rotations for each quad in map
QuadMap genQuadRotations(QuadMap quads, int size) {
	QuadMap out;
	for(std::array<int,5> baseQuad : quads) {
		for(std::array<int,5> quad : genQuadRotations(baseQuad, size))
			if(std::find(out.begin(), out.end(), quad) == out.end())
				out.push_back(quad);
	}
	return out;
}

//Concat 2 square maps
SquareMap operator+(const SquareMap &first, const SquareMap &second) {
	SquareMap third;
	third.insert(third.begin(), first.begin(), first.end());
	third.insert(third.begin(), second.begin(), second.end());
	return third;
}

bool operator==(const std::array<int,10> &lhs, const std::array<int,10> &rhs) {
	for(int i = 0; i < 9; i++)
		if(lhs[i] != rhs[i])
			return false;
	return true;
}

std::ostream& operator<<(std::ostream& os, const std::array<int,10> square) {
	return os <<
		(char)square[0] << (char)square[1] << (char)square[2] << "\n" <<
		(char)square[3] << (char)square[4] << (char)square[5] << "\n" <<
		(char)square[6] << (char)square[7] << (char)square[8] << "\n";
}

/*
0 1 2
3 4 5
6 7 8
*/

//Generate all rotations for a 3x3 square of tiles
SquareMap genSquareRotations(std::array<int,10> square, int size) {
	SquareMap out;
	out.push_back(square);
	//std::cout << out.back() << "\n";

	out.push_back({
		square[6], square[3], square[0],
		square[7], square[4], square[1],
		square[8], square[5], square[2], square[9]+size
	});
	out.push_back({
		square[8], square[7], square[6],
		square[5], square[4], square[3],
		square[2], square[1], square[0], square[9]+size*2
	});
	//out.push_back({square[3], square[2], square[1], square[0], square[4]+size*2});
	out.push_back({
		square[2], square[5], square[8],
		square[1], square[4], square[7],
		square[0], square[3], square[6], square[9]+size*3
	});
	//out.push_back({square[1], square[3], square[0], square[2], square[4]+size*3});

	//Check if horizontal flip = same tile arrangement
	std::array<int,10> flipSquare = {
		square[2], square[1], square[0],
		square[5], square[4], square[3],
		square[8], square[7], square[6], square[9]+size*4
	};
	if(std::find(out.begin(), out.end(), flipSquare) == out.end()) {
		out.push_back(flipSquare);
		out.push_back({
			flipSquare[6], flipSquare[3], flipSquare[0],
			flipSquare[7], flipSquare[4], flipSquare[1],
			flipSquare[8], flipSquare[5], flipSquare[2], flipSquare[9]+size
		});
		out.push_back({
			flipSquare[8], flipSquare[7], flipSquare[6],
			flipSquare[5], flipSquare[4], flipSquare[3],
			flipSquare[2], flipSquare[1], flipSquare[0], flipSquare[9]+size*2
		});
		out.push_back({
			flipSquare[2], flipSquare[5], flipSquare[8],
			flipSquare[1], flipSquare[4], flipSquare[7],
			flipSquare[0], flipSquare[3], flipSquare[6], flipSquare[9]+size*3
		});
	}
	return out;
}

//Generate tile rotations for each 3x3 square in map
SquareMap genSquareRotations(SquareMap squares, int size) {
	SquareMap out;
	for(std::array<int,10> baseSquare : squares) {
		for(std::array<int,10> square : genSquareRotations(baseSquare, size))
			if(std::find(out.begin(), out.end(), square) == out.end())
				out.push_back(square);
	}
	return out;
}

//Read file into list of squares
SquareMap readSquareFile(std::string filename) {
	SquareMap out;

	/*std::string line;
	std::ifstream listFile(filename);
	std::array<int,10> square = {0};

	while(std::getline(listFile, line) && line.size() >= 3) {
		square[0] = line[0];
		square[1] = line[1];
		square[2] = line[2];

		if(std::getline(listFile, line) && line.size() >= 3) {
			square[3] = line[0];
			square[4] = line[1];
			square[5] = line[2];

			if(std::getline(listFile, line) && line.size() >= 3) {
				square[6] = line[0];
				square[7] = line[1];
				square[8] = line[2];
				//std::cout << square << "\n";

				SquareMap rotations = genSquareRotations(square, 0);
				out.insert(out.begin(), rotations.begin(), rotations.end());
				std::getline(listFile, line);
			}
		}
	}
	listFile.close();*/
	return out;
}

void printUniqueSquares(Indexer *indexes) {
	SquareMap out;
	std::vector<int> characters;

	std::cout << "Finding squares\n";
	for(int y = 1; y < indexes->getSize().y-1; y++) {
		for(int x = 1; x < indexes->getSize().x-1; x++) {
			//Add center to character list
			if(std::find(characters.begin(), characters.end(), indexes->getTileI(x,y)) == characters.end())
				characters.push_back(indexes->getTileI(x,y));

			//Create square
			std::array<int,10> baseSquare = {
				indexes->getTileI(x-1,y-1), indexes->getTileI(x,y-1), indexes->getTileI(x+1,y-1),
				indexes->getTileI(x-1,y),   indexes->getTileI(x,y),   indexes->getTileI(x+1,y),
				indexes->getTileI(x-1,y+1), indexes->getTileI(x,y+1), indexes->getTileI(x+1,y+1), 0
			};

			//Add unique square to list
			bool found = false;
			for(std::array<int,10> square : genSquareRotations(baseSquare, 0))
				if(!found && std::find(out.begin(), out.end(), square) != out.end())
					found = true;
			if(!found) {
				out.push_back(baseSquare);
				std::cout << baseSquare << "\n";
			}
		}
	}

	//List unique characters
	std::cout << "Characters: ";
	for(int c : characters)
		std::cout << (char)c;
	std::cout << "\n";

	int t = genSquareRotations(out, 1).size();

	//Count remaining unknown squares
	int s = characters.size();
	std::cout << "Found: " << out.size() << "\n";
	std::cout << "Total: " << t << "\n";
	std::cout << "Max: " << s*s*s*s * s*s*s*s << "\n";
	int h = s+s*(s-1)*3+s*(s-1)*(s-2)*2+s*(s-1)*(s-2)*(s-3);
	std::cout << "Est: " << h*h << "\n";
}