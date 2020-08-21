#pragma once

#include "Node.h"
#include "GridMaker.h"

#include <vector>

enum LightTileType {
	AIR,
	LIGHT,
	SOLID
};

class LightMap : public Node {
private:
	const int tileX;
    const int tileY;
    unsigned int width;
    unsigned int height;

	float ambientIntensity = .3;
	float addedIntensity = 1;
	sf::Color light = sf::Color::White;

	Indexer indexes;
	float** tiles;

	sf::VertexArray vertices;
    sf::RenderTexture *buffer;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		states.transform *= getTransform();
		states.blendMode = sf::BlendMultiply;
		target.draw(vertices, states);
    }

public:
	LightMap(int _tileX, int _tileY, Indexer _indexes, unsigned char layer = 0)
	 : Node(layer), tileX(_tileX), tileY(_tileY), indexes(_indexes) {

	 	//Set sizing
		width = indexes.getSize().x;
	 	height = indexes.getSize().y;
	 	setSize(sf::Vector2i(tileX * width, tileY * height));
        setOrigin(0, 0);

		vertices.setPrimitiveType(sf::Quads);
        vertices.resize((width + 1) * (height + 1) * 4);

		//Build array
		tiles = new float*[height];
		for(unsigned int i = 0; i < height; i++)
			tiles[i] = new float[width];

		//Set up buffer textures
		buffer = new sf::RenderTexture();
        if(!buffer->create(tileX * width, tileY * height))
            throw std::logic_error("Error creating TileMap buffer");

        reload();
	}

	~LightMap() {
		for(unsigned int y = 0; y < height; y++)
			delete[] tiles[y];
		delete[] tiles;
        delete buffer;
    }

	void reload() {
		//Calculate light values
		for(unsigned int i = 0; i < width; ++i)
            for(unsigned int j = 0; j < height; ++j) {
				if(indexes.getTile(sf::Vector2f(i * tileX, j * tileY)) == LIGHT)
					tiles[j][i] = addedIntensity;
				else
					tiles[j][i] = ambientIntensity;
			}

		//Draw lighting
		buffer->clear(sf::Color::White);
		for(unsigned int i = 0; i <= width; ++i)
            for(unsigned int j = 0; j <= height; ++j) {
				sf::Vertex* quad = &vertices[(i + j * width) * 4];

				quad[0].position = getTilePos(i, j);
				quad[1].position = getTilePos(i-1, j);
				quad[2].position = getTilePos(i-1, j-1);
				quad[3].position = getTilePos(i, j-1);

				quad[0].color = applyIntensity(i, j);
				quad[1].color = applyIntensity(i-1, j);
				quad[2].color = applyIntensity(i-1, j-1);
				quad[3].color = applyIntensity(i, j-1);
            }
        buffer->draw(vertices, sf::BlendMultiply);
        buffer->display();
        setTexture(buffer->getTexture());
	}

	sf::Color applyIntensity(unsigned int i, unsigned int j) {
		float intensity = ambientIntensity;
		if(i < width && j < height)
			intensity = tiles[j][i];
		sf::Color result;

		result.a = 255;
		result.r = (char)(light.r * intensity);
		result.g = (char)(light.g * intensity);
		result.b = (char)(light.b * intensity);

		return result;
	}

	sf::Vector2f getTilePos(int x, int y) {
		return sf::Vector2f(tileX / 2.0f + tileX * x, tileY / 2.0f + tileY * y);
	}
};