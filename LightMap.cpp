#include "LightMap.h"

//Based on http://journal.stuffwithstuff.com/2015/09/07/what-the-hero-sees/
class Shadow {
public:
	float start;
	float end;

	Shadow(float _start, float _end) {
		start = _start;
		end = _end;
	}

	/// Returns `true` if [other] is completely covered by this shadow.
	bool contains(Shadow other) {
		return start <= other.start && end >= other.end;
	}
};

class ShadowLine {
private:
	std::vector<Shadow> shadows;

public:

	bool isInShadow(Shadow projection) {
		for(Shadow shadow : shadows)
			if(shadow.contains(projection)) 
				return true;

		return false;
	}

	bool isFullShadow() {
		return shadows.size() == 1 &&
			shadows[0].start == 0 &&
			shadows[0].end == 1;
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
		Shadow *overlappingPrevious = NULL;
		if(index > 0 && shadows[index - 1].end > shadow.start)
			overlappingPrevious = &(shadows[index - 1]);

		Shadow *overlappingNext = NULL;
		if(index < shadows.size() && shadows[index].start < shadow.end)
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
};

// Creates a [Shadow] that corresponds to the projected
// silhouette of the tile at [row], [col].
Shadow projectTile(float row, float col) {
	float topLeft = col / (row + 2);
	float bottomRight = (col + 1) / (row + 1);
	return Shadow(topLeft, bottomRight);
}

sf::Color LightMap::applyIntensity(unsigned int x, unsigned int y) {
	float intensity = ambientIntensity;
	if(x < width && y < height)
		intensity = tiles[x][y];

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
		if(!indexes.inBounds(pos + offset) || maxIntensity < ambientIntensity)
			break;

		float intensity = maxIntensity;

		for(int col = 0; col <= row; col++) {
			pos = light + transformOctant(row, col, octant);

			// If we've traversed out of bounds, bail on this row.
			if(!indexes.inBounds(pos + offset) || intensity < ambientIntensity) 
				break;

			Shadow projection = projectTile(row, col);

			// Set the visibility of this tile.
			bool visible = !line.isInShadow(projection);
			float tileIntensity = visible ? intensity : ambientIntensity;

			if(tileIntensity > tiles[(int)pos.x][(int)pos.y])
				tiles[(int)pos.x][(int)pos.y] = tileIntensity;

			// Add any opaque tiles to the shadow map.
			if(visible && indexes.getTile(pos + offset) < 0) {
				line.add(projection);
				if(line.isFullShadow())
					return;
			}
			intensity -= absorb;
		}
		++row;
		maxIntensity -= absorb;
	}
}

LightMap::LightMap(int _tileX, int _tileY, float _ambient, float _absorb, Indexer _indexes, 
		Layer layer, bool indexLights, sf::Color _lightColor)
		: Node(layer), indexes(_indexes) {

	//Set arguments
	tileX = _tileX;
	tileY = _tileY;
	ambientIntensity = _ambient;
	absorb = _absorb;
	lightColor = _lightColor;

	//Set sizing
	width = indexes.getSize().x * indexes.getScale().x + 1;
	height = indexes.getSize().y * indexes.getScale().y + 1;
	setSize(sf::Vector2i(tileX * width, tileY * height));
	setOrigin(tileX, tileY);

	vertices.setPrimitiveType(sf::Quads);
	vertices.resize((width + 1) * (height + 1) * 4);

	//Build array
	tiles = new float*[width];
	for(unsigned int x = 0; x < width; ++x) {
		tiles[x] = new float[height];
		for(unsigned int y = 0; y < height; ++y) {
			tiles[x][y] = ambientIntensity;

			//Add indexed lights
			if(indexLights) {
				sf::Vector2f pos(x, y);
				if(indexes.getTile(pos + offset) > 0)
					addSource(pos, indexes.getTile(pos + offset) / 100.0);
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
}

sf::VertexArray *LightMap::getVertices() {
	return &vertices;
}

int LightMap::addSource(sf::Vector2f light, float intensity) {
	sourcePosition.push_back(light);
	sourceIntensity.push_back(intensity);
	return sourcePosition.size() - 1;
}

void LightMap::moveSource(int i, sf::Vector2f light) {
	sourcePosition[i] = light;
}

void LightMap::markCollection() {
	singular = false;
	setHidden(true);
}