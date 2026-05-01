#pragma once

#include <iostream>
#include <vector>

//Color type with lots of conversions
class skColor {
public:
	float red = 1;
	float green = 1;
	float blue = 1;
	float alpha = 1;

	skColor() {}

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

	//Read 4 values from list
	skColor(unsigned char *data) : skColor(data[0], data[1], data[2], data[3]) {}
	skColor(int *data) : skColor(data[0], data[1], data[2], data[3]) {}
	skColor(float *data) : skColor(data[0], data[1], data[2], data[3]) {}

	//Read 4 values from list
	template <typename T>
	skColor(std::vector<T> &list, int index=0) : skColor(list[index*4], list[index*4+1], list[index*4+2], list[index*4+3]) {}

	//Integer conversion
	unsigned char r() const {
		return red*255;
	}
	unsigned char g() const {
		return green*255;
	}
	unsigned char b() const {
		return blue*255;
	}
	unsigned char a() const {
		return alpha*255;
	}

	//HSL conversion (Vector.cpp)
	float max() const;
	float min() const;
	float hue() const;
	float saturation() const;
	float luminance() const;
	std::string hex() const;

	//Write 4 values back to list
	void write(int *list, int index=0) {
		list[index*4+0] = r();
		list[index*4+1] = g();
		list[index*4+2] = b();
		list[index*4+3] = a();
	}
	void write(float *list, int index=0) {
		list[index*4+0] = red;
		list[index*4+1] = green;
		list[index*4+2] = blue;
		list[index*4+3] = alpha;
	}

	template <typename T>
	void write(std::vector<T> &list, int index=0) {
		write(list.data(), index);
	}
};

//Color with overriding to skip over alpha more
class skColor3 : public skColor {
public:
	skColor3() {}
	skColor3(float _red, float _green, float _blue, float _alpha=1) : skColor(_red, _green, _blue, _alpha) {}
	skColor3(int _red, int _green, int _blue, int _alpha=255) : skColor(_red, _green, _blue, _alpha) {}

	//Read 3 values from list
	skColor3(unsigned char *data) : skColor(data[0], data[1], data[2]) {}
	skColor3(int *data) : skColor(data[0], data[1], data[2]) {}
	skColor3(float *data) : skColor(data[0], data[1], data[2]) {}

	//Read 3 values from list
	template <typename T>
	skColor3(std::vector<T> &list, int index=0) : skColor(list[index*3], list[index*3+1], list[index*3+2]) {}

	std::string hex() const;

	//Write 3 values back to list
	void write(int *list, int index=0) {
		list[index*3+0] = r();
		list[index*3+1] = g();
		list[index*3+2] = b();
	}
	void write(float *list, int index=0) {
		list[index*3+0] = red;
		list[index*3+1] = green;
		list[index*3+2] = blue;
	}

	template <typename T>
	void write(std::vector<T> &list, int index=0) {
		write(list.data(), index);
	}
};

//Color operators (Vector.cpp)
bool operator==(const skColor &first, const skColor &second);
bool operator!=(const skColor &first, const skColor &second);
std::ostream& operator<<(std::ostream& os, const skColor &color);

//Other conversions
skColor HSLColor(float hue, float saturation, float luminance);
int hexValue(char byte);
int hexValue(char *byte);
skColor hexColor(std::string hex);

//Color constants
const static skColor COLOR_NONE = skColor(1.0f,1.0f,1.0f,0.0f);
const static skColor COLOR_EMPTY = skColor(0.0f,0.0f,0.0f,0.0f);
const static skColor COLOR_WHITE = skColor(1.0f,1.0f,1.0f,1.0f);
const static skColor COLOR_BLACK = skColor(0.0f,0.0f,0.0f,1.0f);
const static skColor COLOR_PURPLE = skColor(200, 122, 255, 255);

//BlendModes
enum SK_BLENDMODE {
	SK_BLEND_NONE,
	SK_BLEND_ALPHA,
	SK_BLEND_ALPHA_MULT,
	SK_BLEND_ADD,
	SK_BLEND_MULT
};