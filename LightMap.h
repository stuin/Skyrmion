#pragma once

#include "DebugLayer.hpp"
#include "GridMaker.h"
#include "Node.h"
#include "UpdateList.h"

#include <vector>

# define M_PI 3.14159265358979323846

class LightMap : public Node {
private:
	const sf::Vector2f offset = sf::Vector2f(0.5,0.5);
	sf::Vector2f tileSize;
	sf::Vector2f tileOffset;
	int tileX;
	int tileY;
	unsigned int width;
	unsigned int height;

	int renderCount = 1;

	//Color values
	float ambientIntensity;
	sf::Color lightColor;
	float absorb;

	//Tile mapping
	float **tiles;
	Indexer indexes;

	//Light sources
	std::vector<sf::Vector2f> sourcePosition;
	std::vector<float> sourceIntensity;
	long unsigned int nextSource = 0;

	//Graphical storage
	std::vector<sf::ConvexShape> lightShapes[2];
	bool swapShapes = false;

	sf::VertexArray vertices;
	int usedQuads = 0;
    sf::RenderTexture buffer;
    Node *collection = NULL;

    std::vector<sf::Vector2f> defaultCirclePoints;
    std::vector<sf::Vector2f> addedCirclePoints;

    DebugLayer *debug = NULL;
    int debugPoints = 0;

	sf::Color applyIntensity(unsigned int x, unsigned int y);
	sf::Color applyIntensity(float intensity);
	sf::Vector2f getTilePos(unsigned int x, unsigned int y);
	sf::Vector2f transformOctant(int row, int col, int octant);
	void createShape(sf::Vector2f pos, std::vector<sf::Vector2f> shape, float intensity);
	void lightOctant(sf::Vector2f light, int octant, float maxIntensity);
	void lightRays(sf::Vector2f light, float maxIntensity);

public:
	LightMap(int _tileX, int _tileY, float _ambient, float _absorb, Indexer _indexes, 
		Layer layer, bool indexLights=true, sf::Color _lightColor=sf::Color::White);

	~LightMap() {
		for(unsigned int y = 0; y < height; y++)
			delete[] tiles[y];
		delete[] tiles;
	}

	void reload();
	void reloadBuffer();

	//Moving lights
	sf::Vector2f scalePosition(sf::Vector2f pos);
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
	LightMapCollection(int tileX, int tileY, Indexer indexes, Layer layer) : Node(layer) {
		int width = tileX * (indexes.getSize().x * indexes.getScale().x + 1);
		int height = tileY * (indexes.getSize().y * indexes.getScale().y + 1);
		blendMode = sf::BlendMultiply;

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