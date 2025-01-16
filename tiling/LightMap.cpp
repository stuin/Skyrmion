#include "LightMap.h"

#include <algorithm>

/*

//Based on http://journal.stuffwithstuff.com/2015/09/07/what-the-hero-sees/
class Shadow {
public:
	float start;
	float end;
	int strength = 0;
	int distance;

	Shadow(float _start, float _end, int _distance) {
		start = _start;
		end = _end;
		distance = _distance;
	}

	/// Returns `true` if [other] is completely covered by this shadow.
	bool covers(Shadow other) {
		return start <= other.start && end >= other.end;
	}

	/// Returns `true` if [other] is partially covered by this shadow.
	bool contains(Shadow other) {
		return start <= other.end && end >= other.start;
	}

	bool contains(Shadow other, bool full) {
		return full ? covers(other) : contains(other);
	}
};

class ShadowLine {
private:
	std::vector<Shadow> shadows;

public:

	float visibility(Shadow projection, bool full) {
		int strength = 0;
		for(Shadow shadow : shadows)
			if(strength < 100 && shadow.contains(projection, full))
				strength += shadow.strength;

		return (100 - std::min(strength, 100)) / 100.0;
	}

	bool isFullShadow() {
		if(shadows.size() == 1)
			return shadows[0].start == 0 &&
			shadows[0].end == 1 && shadows[0].strength == 100;

		float pos = 0;
		for(Shadow shadow : shadows)
			if(shadow.strength == 100 && shadow.start <= pos)
				pos = shadow.end;
		return pos >= 1;
	}

	void add(Shadow shadow) {
		// Figure out where to slot the new shadow in the list.
		long unsigned int index = 0;
		while(index < shadows.size()) {
			// Stop when we hit the insertion point.
			if(shadows[index].start >= shadow.start) 
				break;
			index++;
		}

		// The new shadow is going here. See if it overlaps the
		// previous or next.
		int strength = shadow.strength;
		Shadow *overlappingPrevious = NULL;
		if(index > 0 && shadows[index - 1].end >= shadow.start && shadows[index - 1].strength == strength)
			overlappingPrevious = &(shadows[index - 1]);

		Shadow *overlappingNext = NULL;
		if(index < shadows.size() && shadows[index].start <= shadow.end && shadows[index].strength == strength)
			overlappingNext = &(shadows[index]);

		// Insert and unify with overlapping shadows.
		if(overlappingNext != NULL) {
			if(overlappingPrevious != NULL) {
				// Overlaps both, so unify one and delete the other.
				overlappingPrevious->end = overlappingNext->end;
				shadows.erase(shadows.begin() + index);
			} else {
				// Overlaps the next one, so unify it with that.
				overlappingNext->start = shadow.start;
			}
		} else {
			if(overlappingPrevious != NULL) {
				// Overlaps the previous one, so unify it with that.
				overlappingPrevious->end = shadow.end;
			} else {
				// Does not overlap anything, so insert.
				shadows.insert(shadows.begin() + index, shadow);
			}
		}
	}

	void darkenPartialShadows(float absorb) {
		for(Shadow shadow : shadows)
			if(shadow.strength < 100)
				shadow.strength += absorb * 100;
	}
};

// Creates a [Shadow] that corresponds to the projected
// silhouette of the tile at [row], [col].
Shadow projectTile(float row, float col) {
	float topLeft = col / (row + 2);
	float bottomRight = (col + 1) / (row + 1);
	return Shadow(topLeft, bottomRight, col);
}

sf::Color LightMap::applyIntensity(unsigned int x, unsigned int y) {
	float intensity = ambientIntensity;
	if(x < width && y < height)
		intensity = tiles[x][y];

	return applyIntensity(intensity);
}

//Create color from light percentage
sf::Color LightMap::applyIntensity(float intensity) {
	intensity = std::min(intensity, 1.0f);

	sf::Color result;
	result.a = 255;
	result.r = (char)(lightColor.r * intensity);
	result.g = (char)(lightColor.g * intensity);
	result.b = (char)(lightColor.b * intensity);

	return result;
}

sf::Vector2f LightMap::getTilePos(unsigned int x, unsigned int y) {
	sf::Vector2f pos(tileX / 2.0f + tileX * x, tileY / 2.0f + tileY * y);
	if(x > width + 1)
		pos.x = 0;
	if(y > height + 1)
		pos.y = 0;
	return pos;
}

sf::Vector2f LightMap::transformOctant(int row, int col, int octant) {
	switch(octant) {
		case 0: return sf::Vector2f( col, -row);
		case 1: return sf::Vector2f( row, -col);
		case 2: return sf::Vector2f( row,  col);
		case 3: return sf::Vector2f( col,  row);
		case 4: return sf::Vector2f(-col,  row);
		case 5: return sf::Vector2f(-row,  col);
		case 6: return sf::Vector2f(-row, -col);
		case 7: return sf::Vector2f(-col, -row);
	}
	return sf::Vector2f(0, 0);
}

void LightMap::lightOctant(sf::Vector2f light, int octant, float maxIntensity) {
	ShadowLine line;
	int row = 1;

	while(true) {
		// Stop once we go out of bounds.
		sf::Vector2f pos = light + transformOctant(row, 0, octant);
		if(!indexes->inBounds(pos) || maxIntensity < ambientIntensity)
			break;

		float intensity = maxIntensity;

		for(int col = 0; col <= row; col++) {
			pos = light + transformOctant(row, col, octant);

			// If we've traversed out of bounds, bail on this row.
			if(!indexes->inBounds(pos) || intensity < ambientIntensity)
				break;

			Shadow projection = projectTile(row, col);

			// Set the visibility of this tile.
			float visible = line.visibility(projection, false);
			float tileIntensity = std::max(visible * intensity, ambientIntensity);

			if(tileIntensity > tiles[(int)pos.x][(int)pos.y])
				tiles[(int)pos.x][(int)pos.y] = tileIntensity;

			// Remove shadows on top of lights
			int tileValue = indexes->getTile(pos);
			if(tileValue / 100.0 > tiles[(int)pos.x][(int)pos.y])
				tiles[(int)pos.x][(int)pos.y] = tileValue / 100.0;

			// Add any opaque tiles to the shadow map.
			if(visible > 0 && tileValue < 0) {
				tileValue /= std::sqrt(indexes->getScale().x);
				projection.strength = line.visibility(projection, true)-tileValue;
				line.add(projection);
				if(line.isFullShadow())
					return;
			}
			intensity -= absorb;
		}
		++row;
		maxIntensity -= absorb;
		line.darkenPartialShadows(absorb);
	}
}

LightMap::LightMap(int _tileX, int _tileY, float _ambient, float _absorb, Indexer *_indexes,
		Layer layer, bool indexLights, sf::Color _lightColor)
		: Node(layer), indexes(_indexes) {

	//Set arguments
	tileX = _tileX / indexes->getScale().x;
	tileY = _tileY / indexes->getScale().y;
	tileSize = sf::Vector2f(tileX, tileY);
	ambientIntensity = _ambient;
	absorb = _absorb / indexes->getScale().x;
	lightColor = _lightColor;
	setBlendMode(sf::BlendMultiply);

	//Set sizing
	width = (indexes->getSize().x + 1) * indexes->getScale().x;
	height = (indexes->getSize().y + 1) * indexes->getScale().y;
	setSize(sf::Vector2i(tileX * width, tileY * height));
	setOrigin(-_tileX / 2, -_tileY / 2);

	vertices.setPrimitiveType(sf::Quads);
	vertices.resize((width + 1) * (height + 1) * 4);

	//Set up buffer texture
    if(!buffer.create(tileX * width, tileY * height))
        throw std::logic_error("Error creating LightMap buffer");
    setTexture(buffer.getTexture());

	//Build array
	tiles = new float*[width];
	for(unsigned int x = 0; x < width; ++x) {
		tiles[x] = new float[height];
		for(unsigned int y = 0; y < height; ++y) {
			tiles[x][y] = ambientIntensity;

			//Add static lights
			if(indexLights) {
				sf::Vector2f pos(x, y);
				int tileValue = indexes->getTile(pos);
				if(tileValue > 0)
					addSource(pos * tileSize, tileValue / 100.0);
			}
		}
	}

	reload();
}

void LightMap::reload() {
	//Clear existing lights
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++y)
			tiles[x][y] = ambientIntensity;

	//Propogate Sources
	for(long unsigned int i = 0; i < sourcePosition.size(); i++) {
		sf::Vector2f light = sourcePosition[i];
		if(indexes->inBounds(light))
			tiles[(int)light.x][(int)light.y] = sourceIntensity[i];

		for(int octant = 0; octant < 8; octant++)
			lightOctant(light, octant, sourceIntensity[i]);
	}

	//Draw lighting
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

	UpdateList::scheduleReload(this);
	if(collection != NULL)
		UpdateList::scheduleReload(collection);
}

void LightMap::reloadBuffer() {
	//Update buffer
	buffer.clear(applyIntensity(ambientIntensity));
	buffer.draw(vertices, sf::BlendAdd);
	buffer.display();
	//std::cout << "Redraw lightmap\n";
}

int LightMap::addSource(sf::Vector2f light, float intensity) {
	light = light / tileSize;
	int lastIndex = nextIndex;
	if(nextIndex < sourcePosition.size()) {
		sourcePosition[nextIndex] = light;
		sourceIntensity[nextIndex] = intensity;
		while(nextIndex < sourcePosition.size() && sourceIntensity[nextIndex] > 0)
			++nextIndex;
	} else {
		sourcePosition.push_back(light);
		sourceIntensity.push_back(intensity);
		nextIndex = sourcePosition.size();
	}
	return lastIndex;
}

void LightMap::moveSource(int i, sf::Vector2f light) {
	sourcePosition[i] = light / tileSize;
}

void LightMap::deleteSource(int i) {
	sourceIntensity[i] = 0;
}

void LightMap::markCollection(Node *node) {
	singular = false;
	collection = node;
	setHidden(true);
}
*/