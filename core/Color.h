struct Color {
	float red, green, blue, alpha;

	Color(float _red, float _green, float _blue, float _alpha=1) {
		red = _red;
		green = _green;
		blue = _blue;
		alpha = _alpha;
	}

	Color(int _red, int _green, int _blue, int _alpha=255) {
		red = _red / 255.0;
		green = _green / 255.0;
		blue = _blue / 255.0;
		alpha = _alpha / 255.0;
	}

	Color(unsigned char *data) {
		red = data[0] / 255.0;
		green = data[1] / 255.0;
		blue = data[2] / 255.0;
		alpha = data[3] / 255.0;
	}
};

enum BLENDMODE {
	BLEND_NONE,
	BLEND_ALPHA,
	BLEND_ALPHA_MULT,
	BLEND_ADD,
	BLEND_ADD_MULT,
	BLEND_MOD,
	BLEND_MULT
};