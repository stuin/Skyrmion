#pragma once

#include <vector>

#include "Color.h"
#include "Vector.h"

#define RENDERCOMPONENTERROR std::invalid_argument("Feature not in RenderComponent " + std::to_string(getType()))
#define RENDERCOMPONENTNULL std::invalid_argument("No RenderComponent found")

enum SK_RESOURCE_TYPE {
	SK_INVALID = 0,
	SK_TEXTURE = -1,
	SK_BUFFER = -2,
	SK_SHADER = 1,
	SK_FONT = 2,
	SK_AUDIO = 3,
	SK_TEXT = 4
};

enum SK_RENDER_TYPE {
	RENDER_NONE,
	RENDER_TEXTURE_SINGLE,
	RENDER_TEXTURE_RECT,
	RENDER_TEXTURE_ARRAY,
	RENDER_TEXTURE_MAP,
	RENDER_COLOR_SINGLE,
	RENDER_COLOR_RECT,
	RENDER_COLOR_MAP,
	RENDER_PASSTHROUGH_BUFFER,
	RENDER_STRING
};

class Node;

class RenderComponent {
private:
	Node *source;
	bool hidden = false;

public:
	RenderComponent(Node *_source) {
		this->source = _source;
	}

	Node *getSource() {
		return source;
	}

	bool isHidden() {
		return hidden;
	}
	void setHidden(bool _hidden=true) {
		hidden = _hidden;
	}

	virtual int getType() = 0;
	virtual ~RenderComponent() {}

	//Required getters
	virtual int getBlendMode() = 0;
	virtual sint getTexture() = 0;
	virtual skColor getColor() = 0;

	//Optional getters
	virtual int getPixelSize() { throw new RENDERCOMPONENTERROR; }
	virtual skColor getInsideColor() { throw new RENDERCOMPONENTERROR; }
	virtual TextureRect getTextureRect() { throw new RENDERCOMPONENTERROR; }
	virtual std::vector<TextureRect> *getTextureRects() { throw new RENDERCOMPONENTERROR; }
	virtual RenderComponent *getSubComponent() { throw new RENDERCOMPONENTERROR; }
	virtual const char *getString() { throw new RENDERCOMPONENTERROR; }

	//Optional setters
	virtual void setBlendMode(int blendMode) { throw new RENDERCOMPONENTERROR; }
	virtual void setTexture(sint texture) { throw new RENDERCOMPONENTERROR; }
	virtual void setColor(skColor color) { throw new RENDERCOMPONENTERROR; }
	virtual void setInsideColor(skColor color) { throw new RENDERCOMPONENTERROR; }
	virtual void setPixelSize(int size) { throw new RENDERCOMPONENTERROR; }
	virtual void setTextureRect(TextureRect rectangle, sint i=0) { throw new RENDERCOMPONENTERROR; }
	virtual void setTextureVecRect(Vector2i corner, Vector2i size, sint i=0) { throw new RENDERCOMPONENTERROR; }
	virtual void setTextureIntRect(IntRect rect, sint i=0) { throw new RENDERCOMPONENTERROR; }
	virtual void setSubComponent(int type) { throw new RENDERCOMPONENTERROR; }
	virtual void setSubComponent(RenderComponent *component) { throw new RENDERCOMPONENTERROR; }
	virtual void setString(const char *text) { throw new RENDERCOMPONENTERROR; }
};

RenderComponent *createRenderComponent(int _type, Node *_source);