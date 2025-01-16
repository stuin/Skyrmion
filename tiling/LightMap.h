#pragma once

#include "GridMaker.h"
#include "../core/Node.h"
#include "../core/UpdateList.h"

#include <vector>

/*
 * Generate and show a lightmap based on a grid
 */

/*
class LightMap : public Node {
private:
	const sf::Vector2f offset = sf::Vector2f(-1,-1);
	sf::Vector2f tileSize;
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
	Indexer *indexes;

	//Light sources
	std::vector<sf::Vector2f> sourcePosition;
	std::vector<float> sourceIntensity;
	unsigned int nextIndex = 0;

	//Graphical storage
	sf::VertexArray vertices;
    sf::RenderTexture buffer;
	bool singular = true;
	Node *collection = NULL;

	sf::Color applyIntensity(unsigned int x, unsigned int y);
	sf::Color applyIntensity(float intensity);
	sf::Vector2f getTilePos(unsigned int x, unsigned int y);
	sf::Vector2f transformOctant(int row, int col, int octant);
	void lightOctant(sf::Vector2f light, int octant, float maxIntensity);

public:
	LightMap(int _tileX, int _tileY, float _ambient, float _absorb, Indexer *_indexes,
		Layer layer, bool indexLights=true, sf::Color _lightColor=sf::Color::White);

	~LightMap() {
		for(unsigned int y = 0; y < height; y++)
			delete[] tiles[y];
		delete[] tiles;
	}

	void reload();
	void reloadBuffer();

	//Moving lights
	int addSource(sf::Vector2f light, float intensity);
	void moveSource(int i, sf::Vector2f light);
	void deleteSource(int i);
	void markCollection(Node *node);
};

class LightMapCollection : public Node {
private:
	std::vector<LightMap*> lightmaps;
	sf::RenderTexture *buffer;

public:
	LightMapCollection(int tileX, int tileY, Indexer *indexes, Layer layer) : Node(layer) {
		int width = tileX * (indexes->getSize().x * indexes->getScale().x + 1);
		int height = tileY * (indexes->getSize().y * indexes->getScale().y + 1);
		setBlendMode(sf::BlendMultiply);

		//Set up buffer texture
		buffer = new sf::RenderTexture();
		if(!buffer->create(width, height))
			throw std::logic_error("Error creating LightMap buffer");
		setTexture(buffer->getTexture());

		UpdateList::scheduleReload(this);
	}

	void addLightMap(LightMap *map) {
		map->markCollection(this);
		lightmaps.push_back(map);
		UpdateList::scheduleReload(this);
	}

	void reloadAll() {
		for(unsigned int i = 0; i < lightmaps.size(); i++)
			lightmaps[i]->reload();
	}

	void reloadBuffer() {
		buffer->clear(sf::Color::Black);
		for(unsigned int i = 0; i < lightmaps.size(); i++)
			buffer->draw(*(lightmaps[i]), sf::BlendMax);
		buffer->display();
		//std::cout << "Redraw lightmap collection\n";
	}
};
*/