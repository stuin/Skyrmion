#include "RenderComponent.h"

class TextureSingleRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	sint texture = 0;

public:
	TextureSingleRenderComponent(Node *source) : RenderComponent(source) {}

	int getType() {
		return RENDER_TEXTURE_SINGLE;
	}

	int getBlendMode() {
		return blendMode;
	}
	sint getTexture() {
		return texture;
	}

	skColor getColor() {
		return COLOR_WHITE;
	}

	void setBlendMode(int _blendMode) {
		blendMode = _blendMode;
	}
	void setTexture(sint _texture) {
		texture = _texture;
	}
};

class TextureRectRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	sint texture = 0;
	TextureRect textureRect;

public:
	TextureRectRenderComponent(Node *source) : RenderComponent(source) {}

	int getType() {
		return RENDER_TEXTURE_RECT;
	}

	int getBlendMode() {
		return blendMode;
	}
	sint getTexture() {
		return texture;
	}
	TextureRect getTextureRect() {
		return textureRect;
	}

	skColor getColor() {
		return COLOR_WHITE;
	}

	void setBlendMode(int _blendMode) {
		blendMode = _blendMode;
	}
	void setTexture(sint _texture) {
		texture = _texture;
	}
	void setTextureRect(TextureRect rectangle, sint i=0) {
		textureRect = rectangle;
	}
	void setTextureIntRect(IntRect rect, sint i=0) {
		textureRect = {0, 0, (float)rect.width,(float)rect.height, rect.left,rect.top, rect.width,rect.height, 0};
	}
	void setTextureVecRect(Vector2i corner, Vector2i size, sint i=0) {
		setTextureIntRect(IntRect(corner.x, corner.y, size.x, size.y), i);
	}
};

class TextureArrayRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	sint texture = 0;
	std::vector<TextureRect> textureRects;

public:
	TextureArrayRenderComponent(Node *source) : RenderComponent(source) {}

	int getType() {
		return RENDER_TEXTURE_ARRAY;
	}

	int getBlendMode() {
		return blendMode;
	}
	sint getTexture() {
		return texture;
	}
	TextureRect getTextureRect() {
		return textureRects[0];
	}

	std::vector<TextureRect> *getTextureRects() {
		return &textureRects;
	}

	skColor getColor() {
		return COLOR_WHITE;
	}

	void setBlendMode(int _blendMode) {
		blendMode = _blendMode;
	}
	void setTexture(sint _texture) {
		texture = _texture;
	}
	void setTextureRect(TextureRect rectangle, sint i=0) {
		while(i >= textureRects.size())
			textureRects.emplace_back();
		textureRects[i] = rectangle;
	}
	void setTextureIntRect(IntRect rect, sint i=0) {
		while(i >= textureRects.size())
			textureRects.emplace_back();
		textureRects[i] = {0, 0, (float)rect.width,(float)rect.height, rect.left,rect.top, rect.width,rect.height, 0};
	}

	void setTextureVecRect(Vector2i corner, Vector2i size, sint i=0) {
		setTextureIntRect(IntRect(corner.x, corner.y, size.x, size.y), i);
	}
};

class ColorSingleRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	skColor color = COLOR_WHITE;

public:
	ColorSingleRenderComponent(Node *source) : RenderComponent(source) {}

	int getType() {
		return RENDER_COLOR_SINGLE;
	}

	int getBlendMode() {
		return blendMode;
	}
	sint getTexture() {
		return 0;
	}
	skColor getColor() {
		return color;
	}

	void setBlendMode(int _blendMode) {
		blendMode = _blendMode;
	}
	void setColor(skColor _color) {
		color = _color;
	}
};

class ColorRectRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	int borderWidth = 1;
	skColor insideColor = COLOR_EMPTY;
	skColor outsideColor = COLOR_WHITE;

public:
	ColorRectRenderComponent(Node *source) : RenderComponent(source) {}

	int getType() {
		return RENDER_COLOR_RECT;
	}

	int getBlendMode() {
		return blendMode;
	}
	sint getTexture() {
		return 0;
	}
	skColor getColor() {
		return outsideColor;
	}
	skColor getInsideColor() {
		return insideColor;
	}
	int getPixelSize() {
		return borderWidth;
	}

	void setBlendMode(int _blendMode) {
		blendMode = _blendMode;
	}
	void setColor(skColor _color) {
		outsideColor = _color;
	}
	void setInsideColor(skColor _color) {
		insideColor = _color;
	}
	void setPixelSize(int _size) {
		borderWidth = _size;
	}
};

class StringRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	sint fontTexture = 0;
	int fontSize = 18;
	skColor color = COLOR_WHITE;
	const char *text = NULL;

public:
	StringRenderComponent(Node *source) : RenderComponent(source) {}

	int getType() {
		return RENDER_STRING;
	}

	int getBlendMode() {
		return blendMode;
	}
	sint getTexture() {
		return fontTexture;
	}
	skColor getColor() {
		return color;
	}
	int getPixelSize() {
		return fontSize;
	}

	const char *getString() {
		return text;
	}

	void setBlendMode(int _blendMode) {
		blendMode = _blendMode;
	}
	void setTexture(sint _texture) {
		fontTexture = _texture;
	}
	void setColor(skColor _color) {
		color = _color;
	}
	void setPixelSize(int _size) {
		fontSize = _size;
	}
	void setString(const char *_text) {
		this->text = _text;
	}
};

class PassthroughBufferRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	sint bufferTexture = 0;

	RenderComponent *subComponent = NULL;

public:
	PassthroughBufferRenderComponent(Node *source) : RenderComponent(source) {}

	int getType() {
		return RENDER_PASSTHROUGH_BUFFER;
	}

	int getBlendMode() {
		return blendMode;
	}
	sint getTexture() {
		return bufferTexture;
	}
	RenderComponent *getSubComponent() {
		if(subComponent != NULL)
			return subComponent;
		else
			return this;
	}

	skColor getColor() {
		return COLOR_WHITE;
	}

	void setBlendMode(int _blendMode) {
		blendMode = _blendMode;
	}
	void setTexture(sint _texture) {
		bufferTexture = _texture;
	}
	void setSubComponent(int _type) {
		if(subComponent != NULL)
			delete subComponent;
		subComponent = createRenderComponent(_type, getSource());
	}
	void setSubComponent(RenderComponent *_component) {
		if(subComponent != NULL)
			delete subComponent;
		subComponent = _component;
	}
};

RenderComponent *createRenderComponent(int _type, Node *_source) {
	switch(_type) {
	case RENDER_NONE:
		return NULL;
	case RENDER_TEXTURE_SINGLE:
		return new TextureSingleRenderComponent(_source);
	case RENDER_TEXTURE_RECT:
		return new TextureRectRenderComponent(_source);
	case RENDER_TEXTURE_ARRAY:
		return new TextureArrayRenderComponent(_source);
	case RENDER_COLOR_SINGLE:
		return new ColorSingleRenderComponent(_source);
	case RENDER_COLOR_RECT:
		return new ColorRectRenderComponent(_source);
	case RENDER_STRING:
		return new StringRenderComponent(_source);
	case RENDER_PASSTHROUGH_BUFFER:
		return new PassthroughBufferRenderComponent(_source);
	default:
		return NULL;
	}
}

