#pragma once

#include "GridMaker.h"
#include "../core/Node.h"
#include "../core/UpdateList.h"

#include <vector>

/*
 * Generate and show a lightmap based on a grid
 */

class LightMap : public Node {
private:
	const Vector2f offset = Vector2f(-1,-1);
	Vector2f tileSize;
	int tileX;
	int tileY;
	unsigned int width;
	unsigned int height;

	//Color values
	float ambientIntensity;
	float absorb;
	skColor lightColor;

	//Tile mapping
	float **tiles;
	Indexer *indexes;

	//Light sources
	std::vector<Vector2f> sourcePosition;
	std::vector<float> sourceIntensity;
	unsigned int nextIndex = 0;

	//Graphical storage
	bool singular = true;
	Node *collection = NULL;

	skColor applyIntensity(unsigned int x, unsigned int y);
	skColor applyIntensity(float intensity);
	Vector2f getTilePos(unsigned int x, unsigned int y);
	Vector2f transformOctant(int row, int col, int octant);
	void lightOctant(Vector2f light, int octant, float maxIntensity);

public:
	LightMap(int _tileX, int _tileY, float _ambient, float _absorb, Indexer *_indexes,
		int layer, bool indexLights=true, skColor _lightColor=COLOR_WHITE);

	~LightMap() {
		for(unsigned int y = 0; y < height; y++)
			delete[] tiles[y];
		delete[] tiles;
	}

	void reload();

	//Moving lights
	int addSource(Vector2f light, float intensity);
	void moveSource(int i, Vector2f light);
	void deleteSource(int i);

	void markCollection(Node *node) {
		singular = false;
		collection = node;
		getRenderComponent(false)->setBlendMode(SK_BLEND_MAX);
		setOrigin(0, 0);
		//setHidden(true);
		reload();
	}

};

class LightMapCollection : public Node {
private:
	std::vector<LightMap*> lightmaps;

public:
	LightMapCollection(int tileX, int tileY, Indexer *indexes, int layer, int lightLayer) : Node(layer, RENDER_TEXTURE_SINGLE) {
		int width = tileX * (indexes->getSize().x + 1);
		int height = tileY * (indexes->getSize().y + 1);
		setSize(width, height);
		setOrigin(tileX / 2, tileY / 2);

		//Set up buffer texture
		setTexture(UpdateList::createBuffer(BufferData(0, getSize(), lightLayer, COLOR_BLACK)));
		setBlendMode(SK_BLEND_MULT);
		reload();
	}

	void addLightMap(LightMap *map) {
		map->markCollection(this);
		lightmaps.push_back(map);
		reload();
	}

	void reload() {
		UpdateList::scheduleBufferRefresh(getTexture());
	}

	void reloadAll() {
		for(unsigned int i = 0; i < lightmaps.size(); i++)
			lightmaps[i]->reload();
		reload();
	}
};