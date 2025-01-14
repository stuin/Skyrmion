struct Color {
	float red, green, blue, alpha;

	Color(float _red, float _green, float _blue, float _alpha=1) {
		red = _red;
		green = _green;
		blue = _blue;
		alpha = _alpha;
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