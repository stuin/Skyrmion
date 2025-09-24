enum SK_BLENDMODE {
	SK_BLEND_NONE,
	SK_BLEND_ALPHA,
	SK_BLEND_ALPHA_MULT,
	SK_BLEND_ADD,
	SK_BLEND_MULT
};

enum SK_RESOURCE_TYPE {
	SK_INVALID,
	SK_TEXTURE,
	SK_SHADER,
	SK_FONT,
	SK_AUDIO,
	SK_TEXT
};

struct skColor {
	float red, green, blue, alpha;

	skColor() {
		red = 1;
		green = 1;
		blue = 1;
		alpha = 1;
	}

	skColor(float _red, float _green, float _blue, float _alpha=1) {
		red = _red;
		green = _green;
		blue = _blue;
		alpha = _alpha;
	}

	skColor(int _red, int _green, int _blue, int _alpha=255) {
		red = _red / 255.0;
		green = _green / 255.0;
		blue = _blue / 255.0;
		alpha = _alpha / 255.0;
	}

	skColor(unsigned char *data) {
		red = data[0] / 255.0;
		green = data[1] / 255.0;
		blue = data[2] / 255.0;
		alpha = data[3] / 255.0;
	}

	//Easier format conversion
	unsigned char r() {
		return red*255;
	}
	unsigned char g() {
		return green*255;
	}
	unsigned char b() {
		return blue*255;
	}
	unsigned char a() {
		return alpha*255;
	}
};

bool operator==(const skColor &first, const skColor &second);
bool operator!=(const skColor &first, const skColor &second);

const static skColor COLOR_NONE = skColor(1.0f,1.0f,1.0f,0.0f);
const static skColor COLOR_EMPTY = skColor(0.0f,0.0f,0.0f,0.0f);
const static skColor COLOR_WHITE = skColor(1.0f,1.0f,1.0f,1.0f);
const static skColor COLOR_BLACK = skColor(0.0f,0.0f,0.0f,1.0f);
const static skColor COLOR_PURPLE = skColor(200, 122, 255, 255);