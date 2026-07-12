#include "RenderComponent.h"

class TextureSingleRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	sint texture = 0;
	skColor color = COLOR_WHITE;

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

	skColor getColor(sint i=0) {
		return color;
	}

	void setBlendMode(int _blendMode) {
		blendMode = _blendMode;
	}
	void setTexture(sint _texture) {
		texture = _texture;
	}
	void setColor(skColor _color, sint i=0) {
		color = _color;
	}
};

class TextureRectRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	sint texture = 0;
	skColor color = COLOR_WHITE;
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
	TextureRect *getTextureRect(sint i=0) {
		return &textureRect;
	}

	skColor getColor(sint i=0) {
		return color;
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
	void setColor(skColor _color, sint i=0) {
		color = _color;
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
	TextureRect *getTextureRect(sint i=0) {
		return &textureRects[i];
	}

	std::vector<TextureRect> *getTextureRects() {
		return &textureRects;
	}

	skColor getColor(sint i=0) {
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
	skColor getColor(sint i=0) {
		return color;
	}

	void setBlendMode(int _blendMode) {
		blendMode = _blendMode;
	}
	void setColor(skColor _color, sint i=0) {
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
	skColor getColor(sint i=0) {
		if(i == 0)
			return outsideColor;
		else
			return insideColor;
	}
	int getSize() {
		return borderWidth;
	}

	void setBlendMode(int _blendMode) {
		blendMode = _blendMode;
	}
	void setColor(skColor _color, sint i=0) {
		if(i == 0)
			outsideColor = _color;
		else
			insideColor = _color;
	}
	void setSize(int _size) {
		borderWidth = _size;
	}
};

class ColorArrayRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	sint texture = 0;
	int width = 1;
	std::vector<skColor> colors;

public:
	ColorArrayRenderComponent(Node *source) : RenderComponent(source) {}

	int getType() {
		return RENDER_COLOR_ARRAY;
	}

	int getBlendMode() {
		return blendMode;
	}
	sint getTexture() {
		return texture;
	}
	skColor getColor(sint i=0) {
		return colors[i];
	}
	std::vector<skColor> *getColors() {
		return &colors;
	}
	int getSize() {
		return width;
	}

	void setBlendMode(int _blendMode) {
		blendMode = _blendMode;
	}
	void setTexture(sint _texture) {
		texture = _texture;
	}
	void setColor(skColor _color, sint i=0) {
		colors[i] = _color;
	}
	void setSize(int _size) {
		width = _size;
	}
};

class GradientArrayRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	sint texture = 0;
	int width = 1;
	std::vector<skColor> colors;

public:
	GradientArrayRenderComponent(Node *source) : RenderComponent(source) {}

	int getType() {
		return RENDER_GRADIENT_ARRAY;
	}

	int getBlendMode() {
		return blendMode;
	}
	sint getTexture() {
		return texture;
	}
	skColor getColor(sint i=0) {
		if(i < colors.size())
			return colors[i];
		return COLOR_PURPLE;
	}
	std::vector<skColor> *getColors() {
		return &colors;
	}
	int getSize() {
		return width;
	}

	void setBlendMode(int _blendMode) {
		blendMode = _blendMode;
	}
	void setTexture(sint _texture) {
		texture = _texture;
	}
	void setColor(skColor _color, sint i=0) {
		while(i >= colors.size())
			colors.emplace_back();
		colors[i] = _color;
	}
	void setSize(int _size) {
		width = _size;
	}
};

class ColorTextureArrayRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	sint texture = 0;
	std::vector<TextureRect> textureRects;
	std::vector<skColor> colors;

public:
	ColorTextureArrayRenderComponent(Node *source) : RenderComponent(source) {}

	int getType() {
		return RENDER_COLOR_TEXTURE_ARRAY;
	}

	int getBlendMode() {
		return blendMode;
	}
	sint getTexture() {
		return texture;
	}
	TextureRect *getTextureRect(sint i=0) {
		return &textureRects[i];
	}
	std::vector<TextureRect> *getTextureRects() {
		return &textureRects;
	}

	skColor getColor(sint i=0) {
		if(i < colors.size())
			return colors[i];
		return COLOR_PURPLE;
	}
	std::vector<skColor> *getColors() {
		return &colors;
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

	void setColor(skColor _color, sint i=0) {
		while(i >= colors.size())
			colors.emplace_back();
		colors[i] = _color;
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
	skColor getColor(sint i=0) {
		return color;
	}
	int getSize() {
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
	void setColor(skColor _color, sint i=0) {
		color = _color;
	}
	void setSize(int _size) {
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

	skColor getColor(sint i=0) {
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
	case RENDER_COLOR_ARRAY:
		return new ColorArrayRenderComponent(_source);
	case RENDER_GRADIENT_ARRAY:
		return new GradientArrayRenderComponent(_source);
	case RENDER_COLOR_TEXTURE_ARRAY:
		return new ColorTextureArrayRenderComponent(_source);
	case RENDER_STRING:
		return new StringRenderComponent(_source);
	case RENDER_PASSTHROUGH_BUFFER:
		return new PassthroughBufferRenderComponent(_source);
	default:
		return NULL;
	}
}

