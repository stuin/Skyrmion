#include "VertexGraph.hpp"
#include "../core/Node.h"
#include "../input/MovementEnums.h"

#define PARENTEDGE 4
#define CHILDEDGE 5

class RectangleVertex : public Vertex<6> {
public:
	Vector2i minSize;
	IntRect rect;
	Vector2i padding;
	Vector2i margin;

	RectangleVertex(RectangleVertex *_parent, Vector2i _minSize,
		Vector2i _padding=Vector2i(), Vector2i _margin=Vector2i()) : Vertex(_parent) {

		minSize = _minSize;
		rect = IntRect(Vector2i(), _minSize);
		padding = _padding;
		margin = _margin;
	}

	int reverseDir(int dir) {
		if(dir % 2 == 0)
			return dir + 1;
		return dir - 1;
	}

	Vector2i nextPos(int dir, Vector2i otherSize) {
		switch(dir) {
		case UP:
			return Vector2i(rect.left, rect.top-otherSize.y+padding.y);
		case DOWN:
			return Vector2i(rect.left, rect.top+rect.height+padding.y);
		case LEFT:
			return Vector2i(rect.left-otherSize.x+padding.x, rect.top);
		case RIGHT:
			return Vector2i(rect.left+rect.width+padding.x, rect.top);
		case PARENTEDGE:
			return Vector2i(rect.left-margin.x, rect.top-margin.y);
		case CHILDEDGE:
			return Vector2i(rect.left+margin.x, rect.top+margin.y);
		}
		return rect.pos();
	}

	Vector2i propogateSize(int dir, Vector2i size) {
		if((dir == UP || dir == DOWN) && size.x != rect.width) {
			rect.width = std::max(rect.width, size.x);
			size.x = rect.width;
		} else if((dir == LEFT || dir == RIGHT) && size.y != rect.height) {
			rect.height = std::max(rect.height, size.y);
			size.y = rect.height;
		}

		if(getVertex(dir) != NULL && !hasEitherParent(getVertex(dir)))
			((RectangleVertex*)getVertex(dir))->propogateSize(dir, size);
		else if(dir == DOWN || dir == RIGHT)
			return rect.pos() + rect.size();
		return rect.pos();
	}

	void propogateSize() {
		Vector2i end = min(propogateSize(UP, rect.size()), propogateSize(LEFT, rect.size()));
		Vector2i start = min(propogateSize(DOWN, rect.size()), propogateSize(RIGHT, rect.size()));

		if(getVertex(PARENTEDGE) != NULL)
			((RectangleVertex*)getVertex(PARENTEDGE))->setRect(start, end, true);
	}

	IntRect getRect() {
		return rect;
	}

	void setRect(Vector2i _start, Vector2i _end, bool useMargin=false) {
		Vector2i start = min(_start, _end);
		Vector2i end = max(_start, _end);

		IntRect newRect = IntRect(start, end-start);
		if(useMargin)
			newRect = IntRect(start-margin, end-start+margin);
		rect = IntRect(newRect.pos(), max(rect.size(), newRect.size()));

		propogateSize();
	}

	void setMinSize(Vector2i _minSize) {
		minSize = _minSize;
		rect = IntRect(rect.pos(), max(rect.size(), _minSize));
		propogateSize();
	}

	void setPos(Vector2i _pos) {
		rect.left = _pos.x;
		rect.top = _pos.y;
	}

	RectangleVertex *addRect(int dir, RectangleVertex *vertex) {
		addVertex(dir, reverseDir(dir), vertex);
		vertex->setPos(nextPos(dir, vertex->getRect().size()));

		if(dir < PARENTEDGE) {
			Vector2i end = vertex->propogateSize(dir, rect.size());
			Vector2i start = vertex->propogateSize(reverseDir(dir), vertex->rect.size());

			if(getVertex(PARENTEDGE) != NULL) {
				vertex->setVertex(PARENTEDGE, getVertex(PARENTEDGE));
				((RectangleVertex*)getVertex(PARENTEDGE))->setRect(start, end, true);
			}
		} else if(dir == CHILDEDGE)
			setRect(rect.pos(), rect.pos()+rect.size(), true);
		return vertex;
	}

	RectangleVertex *addRect(int dir, Vector2i size) {
		return addRect(dir, new RectangleVertex(this, size, padding, margin));
	}

	RectangleVertex *addChild(int dir, RectangleVertex *vertex) {
		vertex->setVParent(this);
		if(getVertex(CHILDEDGE) == NULL)
			return addRect(CHILDEDGE, vertex);
		return ((RectangleVertex*)getVertex(CHILDEDGE))->addRect(dir, vertex);
	}
};