#include "LightMap.h"

#include <algorithm>

sf::Vector2f intersect(sf::Vector2f start1, sf::Vector2f end1, sf::Vector2f start2, sf::Vector2f end2) {
    double a1 = end1.y - start1.y;
    double b1 = start1.x - end1.x;
    double c1 = a1*(start1.x) + b1*(start1.y);
 
    double a2 = end2.y - start2.y;
    double b2 = start2.x - end2.x;
    double c2 = a2*(start2.x)+ b2*(start2.y);
 
    double determinant = a1*b2 - a2*b1;
    if(determinant < 1e-6) {
        return sf::Vector2f(0,0);
    }

    double x = (b2*c1 - b1*c2)/determinant;
    double y = (a1*c2 - a2*c1)/determinant;
    return sf::Vector2f(x, y);
}

float distance(sf::Vector2f start, sf::Vector2f end=sf::Vector2f(0,0)) {
	return std::sqrt(std::pow(end.x - start.x, 2) + std::pow(end.y - start.y, 2));
}

sf::Vector2f centerPoint(sf::Vector2f pos, sf::Vector2f tileSize) {
	pos.x = std::floor(pos.x / tileSize.x) * tileSize.x + tileSize.x / 2;
	pos.y = std::floor(pos.y / tileSize.y) * tileSize.y + tileSize.y / 2;
	return pos;
}

sf::Vector2f centerToEdge(sf::Vector2f dir, sf::Vector2f tileSize) {
	dir = vectorLength(dir, 1);
	tileSize /= 2.0f;
	if(dir.x == 0 || dir.y == 0)
		return dir * tileSize;

	float magnitude = 1;
	if(tileSize.x/fabs(dir.x) <= tileSize.y/fabs(dir.y))
		magnitude = fabs(tileSize.x/fabs(dir.x));
	else
		magnitude = tileSize.y/fabs(dir.y);

	return sf::Vector2f(dir.x*magnitude, dir.y*magnitude);
}

sf::Color LightMap::applyIntensity(unsigned int x, unsigned int y) {
	float intensity = ambientIntensity;
	if(x < width && y < height)
		intensity = tiles[x][y];

	return applyIntensity(intensity);
}

sf::Color LightMap::applyIntensity(float intensity) {
	sf::Color result;
	result.a = 255;
	result.r = (char)(lightColor.r * intensity);
	result.g = (char)(lightColor.g * intensity);
	result.b = (char)(lightColor.b * intensity);

	return result;
}

void LightMap::createShape(sf::Vector2f pos, std::vector<sf::Vector2f> shape, float intensity) {
	sf::Vector2f scale = getScale();
	int size = shape.size();
	int shapesI = swapShapes ? 1 : 0;
	//size = renderCount++ % (size - 2) + 2;

	sf::ConvexShape polygon;
	polygon.setPointCount(size);
	for(int i = 0; i < size; i++)
		polygon.setPoint(i, shape[i]);
	polygon.setPosition(pos + tileOffset);
	polygon.setFillColor(applyIntensity(intensity));

	if(false) {
		polygon.setOutlineColor(sf::Color::White);
		polygon.setOutlineThickness(1);
		polygon.setFillColor(applyIntensity(0));
	}
	lightShapes[shapesI].push_back(polygon);
	usedQuads += 1;
}

void LightMap::lightRays(sf::Vector2f light, float maxIntensity) {
	int maxRange = std::ceil(maxIntensity / absorb) * tileX;

	debugPoints = 0;
	//debug->addDot(light2);

	std::vector<sf::Vector2f> points;
	for(sf::Vector2f point : defaultCirclePoints) {
		float r = 1;
		float length = distance(point);
		while(r * length < maxRange && indexes.getTile(light + point*r) == 0)
			r += 1;

		sf::Vector2f edge = sf::Vector2f(0,0);
		if(r * length < maxRange) {
			sf::Vector2f square = centerPoint(light + point*r, tileSize);
			sf::Vector2f offset = ((light + point*r) - square) * 2.0f;
			offset.x = fabs(offset.x);
			offset.y = fabs(offset.y);
			edge = centerToEdge(point, tileSize + offset);
			//debug->addDot(light + point);
			//debug->addDot(square);
			//debug->addDot(light + point*r + edge);
			if(indexes.inBounds(light + point*r + edge) && distance(point*r + edge) < maxRange)
				points.push_back(point*r + edge);
			else
				points.push_back(vectorLength(point, maxRange));
		} else
			points.push_back(vectorLength(point, maxRange));
	}

	createShape(light, points, maxIntensity);
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
	blendMode = sf::BlendMultiply;

	tileSize = sf::Vector2f(tileX, tileY);
	tileOffset = tileSize * offset;

	//Set sizing
	width = indexes.getSize().x * indexes.getScale().x + 1;
	height = indexes.getSize().y * indexes.getScale().y + 1;
	setSize(sf::Vector2i(tileX * width, tileY * height));
	setOrigin(0, 0);

	debug = new DebugLayer("res/small_pixel.ttf", getSize(), layer);
	UpdateList::addNode(debug);

	vertices.setPrimitiveType(sf::Quads);
	//vertices.resize((width + 1) * (height + 1) * 4);

	for(float a = -1; a < 1; a += 0.02)
		defaultCirclePoints.push_back(centerToEdge(sf::Vector2f(cos(a*M_PI), sin(a*M_PI)), tileSize));

	//Set up buffer texture
    if(!buffer.create(tileX * width, tileY * height))
        throw std::logic_error("Error creating LightMap buffer");
    setTexture(buffer.getTexture());

    //Add static lights
    if(indexLights) {
    	indexes.mapGrid([this](char c, sf::Vector2f pos) {
    		int tileValue = this->indexes.getTile(c);
			if(tileValue > 0)
				this->addSource(pos + this->tileOffset, tileValue / 100.0);
		});
    }

	reload();
}

void LightMap::reload() {
	int shapesI = swapShapes ? 1 : 0;
	vertices.clear();
	lightShapes[shapesI].clear();
	debug->clear();
	usedQuads = 0;

	//Propogate Sources
	for(long unsigned int i = 0; i < sourcePosition.size(); i++) {
		sf::Vector2f light = sourcePosition[i];

		//for(int octant = 0; octant < 2; octant++)
		//	lightOctant(light, octant, sourceIntensity[i]);
		lightRays(light, sourceIntensity[i]);
	}
	vertices.resize(usedQuads * 4);
	swapShapes = !swapShapes;

	UpdateList::scheduleReload(this);
	if(collection != NULL)
		UpdateList::scheduleReload(collection);
}

void LightMap::reloadBuffer() {
	int shapesI = swapShapes ? 0 : 1;

	//Update buffer
	buffer.clear(applyIntensity(ambientIntensity));
	for(sf::ConvexShape s : lightShapes[shapesI])
		buffer.draw(s, sf::BlendAdd);
	buffer.display();
	//std::cout << "Redraw lightmap\n";
}

int LightMap::addSource(sf::Vector2f light, float intensity) {
	int lastIndex = nextSource;
	if(nextSource < sourcePosition.size()) {
		sourcePosition[nextSource] = light;
		sourceIntensity[nextSource] = intensity;
		while(nextSource < sourcePosition.size() && sourceIntensity[nextSource] > 0)
			++nextSource;
	} else {
		sourcePosition.push_back(light);
		sourceIntensity.push_back(intensity);
		nextSource = sourcePosition.size();
	}
	return lastIndex;
}

void LightMap::moveSource(int i, sf::Vector2f light) {
	sourcePosition[i] = light;
}

void LightMap::deleteSource(int i) {
	sourceIntensity[i] = 0;
}

void LightMap::markCollection(Node *_collection) {
	collection = _collection;
	setHidden(true);
}