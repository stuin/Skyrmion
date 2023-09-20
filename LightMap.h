#pragma once

#include "Node.h"
#include "GridMaker.h"

#include <vector>

class LightMap : public Node {
private:
	const sf::Vector2f offset = sf::Vector2f(-1,-1);
	int tileX;
	int tileY;
	unsigned int width;
	unsigned int height;

	//Color values
	float ambientIntensity;
	float absorb;
	sf::Color lightColor;

	//Tile mapping
	float **tiles;
	Indexer indexes;

	//Light sources
	std::vector<sf::Vector2f> sourcePosition;
	std::vector<float> sourceIntensity;

	//Graphical storage
	sf::VertexArray vertices;
	bool singular = true;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		states.transform *= getTransform();
		if(singular)
			states.blendMode = sf::BlendMultiply;
		target.draw(vertices, states);
	}

	sf::Color applyIntensity(unsigned int x, unsigned int y);
	sf::Vector2f getTilePos(unsigned int x, unsigned int y);
	sf::Vector2f transformOctant(int row, int col, int octant);
	void lightOctant(sf::Vector2f light, int octant, float maxIntensity);

public:
	LightMap(int _tileX, int _tileY, float _ambient, float _absorb, Indexer _indexes, 
		Layer layer, bool indexLights=true, sf::Color _lightColor=sf::Color::White);

	~LightMap() {
		for(unsigned int y = 0; y < height; y++)
			delete[] tiles[y];
		delete[] tiles;
	}

	void reload();
	sf::VertexArray *getVertices();

	//Moving lights
	int addSource(sf::Vector2f light, float intensity);
	void moveSource(int i, sf::Vector2f light);
	void markCollection();
};

class LightMapCollection : public Node {
private:
	std::vector<LightMap*> lightmaps;
	sf::RenderTexture *buffer;

	//Draw selected tilemap
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		states.transform *= getTransform();
		states.blendMode = sf::BlendMultiply;

		buffer->clear(sf::Color::Black);
		for(unsigned int i = 0; i < lightmaps.size(); i++)
			buffer->draw(*(lightmaps[i]), sf::BlendAdd);
		buffer->display();

		sf::Sprite sprite(buffer->getTexture());
		target.draw(sprite, states);
	}

public:
	LightMapCollection(int tileX, int tileY, Indexer indexes, Layer layer) : Node(layer) {
		int width = tileX * (indexes.getSize().x * indexes.getScale().x + 1);
		int height = tileY * (indexes.getSize().y * indexes.getScale().y + 1);

		//Set up buffer texture
		buffer = new sf::RenderTexture();
		if(!buffer->create(width, height))
			throw std::logic_error("Error creating LightMap buffer");
	}

	void addLightMap(LightMap *map) {
		map->markCollection();
		lightmaps.push_back(map);
	}

	void reloadAll() {
		for(unsigned int i = 0; i < lightmaps.size(); i++)
			lightmaps[i]->reload();
	}
};