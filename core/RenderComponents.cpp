#include "RenderComponent.h"

class SingleTextureRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	sint texture = 0;

public:
	int getType() {
		return RENDER_SINGLE_TEXTURE;
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

class SingleBufferRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	sint texture = 0;

public:
	int getType() {
		return RENDER_SINGLE_BUFFER;
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

class SingleColorRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	skColor color = COLOR_WHITE;

public:
	int getType() {
		return RENDER_SINGLE_COLOR;
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

class TextureRectRenderComponent : public RenderComponent {
private:
	int blendMode = 1;
	sint texture = 0;
	TextureRect textureRect;

public:
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

	//Create rectangle borders from one pixel of texture
	void createPixelRect(FloatRect rect, Vector2i pixel, sint i) {
		setTextureRect({rect.left,rect.top, 			rect.width,1,  pixel.x,pixel.y, 1,1,0}, i+0);
		setTextureRect({rect.left,rect.top+rect.height,	rect.width,1,  pixel.x,pixel.y, 1,1,0}, i+1);
		setTextureRect({rect.left,rect.top, 			1,rect.height, pixel.x,pixel.y, 1,1,0}, i+2);
		setTextureRect({rect.left+rect.width,rect.top, 	1,rect.height, pixel.x,pixel.y, 1,1,0}, i+3);
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
	int getFontSize() {
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
	void setFontSize(int _size) {
		fontSize = _size;
	}
	void setColor(skColor _color) {
		color = _color;
	}
	void setString(const char *_text) {
		this->text = _text;
	}
};

RenderComponent *createRenderComponent(int _type) {
	switch(_type) {
	case RENDER_NONE:
		return NULL;
	case RENDER_SINGLE_TEXTURE:
		return new SingleTextureRenderComponent();
	case RENDER_SINGLE_BUFFER:
		return new SingleBufferRenderComponent();
	case RENDER_SINGLE_COLOR:
		return new SingleColorRenderComponent();
	case RENDER_TEXTURE_RECT:
		return new TextureRectRenderComponent();
	case RENDER_TEXTURE_ARRAY:
		return new TextureArrayRenderComponent();
	case RENDER_STRING:
		return new StringRenderComponent();
	default:
		return NULL;
	}
}

