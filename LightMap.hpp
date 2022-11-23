#pragma once

#include "Node.h"
#include "GridMaker.h"

#include <vector>
#include <unordered_set>
#include <utility>

// A hash function used to hash a pair of any kind
struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const std::pair<T1, T2>& p) const {
        auto hash1 = std::hash<T1>{}(p.first);
        auto hash2 = std::hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

class LightMap : public Node {
private:
	const int tileX;
    const int tileY;
    unsigned int width;
    unsigned int height;

    //Color values
	const float ambientIntensity = 0.4;
	const float absorb = 0.1;
	const sf::Color light = sf::Color::White;

	//Tile mapping
	float** tiles;
	std::vector<std::pair<unsigned int, unsigned int>> sources;
	std::unordered_set<std::pair<unsigned int, unsigned int>, hash_pair> solids;

	//Graphical storage
	sf::VertexArray vertices;
    sf::RenderTexture *buffer;
    sf::Sprite sprite;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		states.transform *= getTransform();
		states.blendMode = sf::BlendMultiply;
		target.draw(sprite, states);
    }

    sf::Color applyIntensity(unsigned int x, unsigned int y) {
		float intensity = ambientIntensity;
		if(x < width && y < height)
			intensity = tiles[y][x];
		sf::Color result;

		result.a = 255;
		result.r = (char)(light.r * intensity);
		result.g = (char)(light.g * intensity);
		result.b = (char)(light.b * intensity);

		return result;
	}

	sf::Vector2f getTilePos(unsigned int x, unsigned int y) {
		sf::Vector2f pos(tileX / 2.0f + tileX * x, tileY / 2.0f + tileY * y);
		if(x > width + 1)
			pos.x = 0;
		if(y > height + 1)
			pos.y = 0;
		return pos;
	}

	void spreadSideways(unsigned int _x, unsigned int y) {
		//Rightwards loop
		unsigned int x = _x;
		float intensity = tiles[y][x];
		while(intensity > ambientIntensity && ++x < width) {
			if(tiles[y][x] >= intensity || solids.count(std::pair(x, y)) == 1)
				break;
			tiles[y][x] = (intensity -= absorb);
		}

		//Leftwards loop
		x = _x;
		intensity = tiles[y][x];
		while(intensity > ambientIntensity && --x < width) {
			if(tiles[y][x] >= intensity || solids.count(std::pair(x, y)) == 1)
				break;
			tiles[y][x] = (intensity -= absorb);
		}
	}

public:
	LightMap(int _tileX, int _tileY, int _width, int _height, unsigned char layer = 0)
	 : Node(layer), tileX(_tileX), tileY(_tileY), width(_width + 1), height(_height + 1) {

	 	//Set sizing
	 	setSize(sf::Vector2i(tileX * width, tileY * height));
        setOrigin(tileX, tileY);

		vertices.setPrimitiveType(sf::Quads);
        vertices.resize((width + 1) * (height + 1) * 4);

		//Build array
		tiles = new float*[height];
		for(unsigned int y = 0; y < height; ++y) {
			tiles[y] = new float[width];
			for(unsigned int x = 0; x < width; ++x)
				tiles[y][x] = ambientIntensity;
		}

		//Set up buffer textures
		buffer = new sf::RenderTexture();
        if(!buffer->create(tileX * width, tileY * height))
            throw std::logic_error("Error creating TileMap buffer");
	}

	~LightMap() {
		for(unsigned int y = 0; y < height; y++)
			delete[] tiles[y];
		delete[] tiles;
        delete buffer;
    }

	void reload() {
		//Propogate Sources
		for(auto point : sources) {
			unsigned int x = point.first;
			unsigned int y = point.second;
			spreadSideways(x, y);

			//Downwards loop
			float intensity = tiles[y][x];
			while(intensity > ambientIntensity && ++y < height) {
				if(tiles[y][x] >= intensity || solids.count(std::pair(x, y)) == 1)
					break;
				tiles[y][x] = (intensity -= absorb);
				spreadSideways(x, y);
			}

			//Upwards loop
			y = point.second;
			intensity = tiles[y][x];
			while(intensity > ambientIntensity && --y < height) {
				if(tiles[y][x] >= intensity || solids.count(std::pair(x, y)) == 1)
					break;
				tiles[y][x] = (intensity -= absorb);
				spreadSideways(x, y);
			}
		}

		//Draw lighting
		buffer->clear(sf::Color::White);
		for(unsigned int x = 0; x < width + 1; ++x)
            for(unsigned int y = 0; y < height + 1; ++y) {
				sf::Vertex* quad = &vertices[(x + y * width) * 4];

				quad[0].position = getTilePos(x, y);
				quad[1].position = getTilePos(x-1, y);
				quad[2].position = getTilePos(x-1, y-1);
				quad[3].position = getTilePos(x, y-1);

				quad[0].color = applyIntensity(x, y);
				quad[1].color = applyIntensity(x-1, y);
				quad[2].color = applyIntensity(x-1, y-1);
				quad[3].color = applyIntensity(x, y-1);
            }
        buffer->draw(vertices, sf::BlendMultiply);
        buffer->display();
        sprite.setTexture(buffer->getTexture());
	}

	void addSource(unsigned int x, unsigned int y, float intensity) {
		sources.push_back(std::pair(x + 1, y + 1));
		tiles[y + 1][x + 1] = intensity;
	}

	void addSolid(unsigned int x, unsigned int y) {
		solids.insert(std::pair(x + 1, y + 1));
	}
};