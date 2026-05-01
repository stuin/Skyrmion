#include "Vector.h"
#include "Color.h"
#include "Event.h"

Vector2f operator*(const Vector2f &first, const Vector2i &second) {
	return Vector2f(first.x * second.x, first.y * second.y);
}
Vector2f operator*(const Vector2i &first, const float second) {
	return Vector2f(first.x * second, first.y * second);
}

Vector2f operator/(const Vector2f &first, const Vector2i &second) {
	return Vector2f(first.x / second.x, first.y / second.y);
}

std::ostream& operator<<(std::ostream& os, const Vector2f &pos) {
	return os << "(" << std::to_string(pos.x) << "," << std::to_string(pos.y) << ") ";
}
std::ostream& operator<<(std::ostream& os, const Vector2i &pos) {
	return os << "(" << std::to_string(pos.x) << "," << std::to_string(pos.y) << ") ";
}

//Create a vector with fixed length in any direction
Vector2f vectorLength(Vector2f dir, double distance) {
	float xOffset = 0;
	float yOffset = 0;
	if(dir.x == 0 && dir.y == 0)
		return Vector2f(0, 0);
	if(dir.x == 0)
		yOffset = copysign(distance, dir.y);
	else if(dir.y == 0)
		xOffset = copysign(distance, dir.x);
	else if(abs(dir.x) == abs(dir.y)) {
		xOffset = RT2O2 * copysign(distance, dir.x);
		yOffset = RT2O2 * copysign(distance, dir.y);
	} else {
		float angle = std::atan2(dir.y, dir.x);
		xOffset = cos(angle) * distance;
		yOffset = sin(angle) * distance;
	}

	return Vector2f(xOffset, yOffset);
}

//Get length of a vector or distance between points
float distance(Vector2f start, Vector2f end) {
	return std::sqrt(std::pow(end.x - start.x, 2) + std::pow(end.y - start.y, 2));
}

Vector2i round(Vector2f pos) {
	return Vector2i(round(pos.x), round(pos.y));
}

bool operator==(const TextureRect &first, const TextureRect &second) {
	return first.px == second.px && first.py == second.py && first.tx == second.tx && first.ty == second.ty;
}

//Event operators
bool operator==(const Event &first, const Event &second) {
    return first.type == second.type && first.down == second.down && first.code == second.code &&
        first.x == second.x && first.y == second.y;
}
bool operator!=(const Event &first, const Event &second) {
    return !(first == second);
}

std::ostream& operator<<(std::ostream& os, const Event &e) {
    return os << std::to_string(e.type) << ':' << std::to_string(e.down) << ':' << std::to_string(e.code) <<
        "(" << std::to_string(e.x) << ',' << std::to_string(e.y) << ") ";
}

//skColor operators
bool operator==(const skColor &first, const skColor &second) {
	return first.red == second.red && first.green == second.green && first.blue == second.blue && first.alpha == second.alpha;
}
bool operator!=(const skColor &first, const skColor &second) {
	return !(first == second);
}
std::ostream& operator<<(std::ostream& os, const skColor &color) {
	return os << color.hex();
}

//HSL conversion
float skColor::max() const {
	return std::max(red, std::max(green, blue));
}
float skColor::min() const {
	return std::min(red, std::min(green, blue));
}
float skColor::hue() const {
	float m = max();
	float d = m-min();
	if(d == 0)
		return 0;
	else if(m == red)
		return std::fmod((green-blue)/d,6) * 60;
	else if(m == green)
		return (((blue-red)/d)+2) * 60;
	else
		return (((red-green)/d)+4) * 60;
}
float skColor::saturation() const {
	float d = max()-min();
	if(d == 0)
		return 0;
	return d/(1-abs(2*luminance()-1));
}
float skColor::luminance() const {
	return (max()+min())/2;
}

skColor HSLColor(float hue, float saturation, float luminance) {
	float c = (1 - abs(2 * luminance - 1)) * saturation;
	float x = c * (1 - abs(std::fmod(hue / 60, 2) - 1));
	float m = luminance - c/2;
	int h = hue/60;
	c += m;
	x += m;

	switch(h) {
	case 0:
		return skColor(c,x,m);
	case 1:
		return skColor(x,c,m);
	case 2:
		return skColor(m,c,x);
	case 3:
		return skColor(m,x,c);
	case 4:
		return skColor(x,m,c);
	case 5:
		return skColor(c,m,x);
	}
	return skColor();
}

//Hex code conversion
std::string skColor::hex() const {
	char s[10];
	sprintf(s, "#%X%X%X%X", r(), g(), b(), a());
	return std::string(s);
}
std::string skColor3::hex() const {
	char s[8];
	sprintf(s, "#%X%X%X", r(), g(), b());
	return std::string(s);
}

int hexValue(char byte) {
	if(byte >= '0' && byte <= '9')
		return byte - '0';
    else if(byte >= 'a' && byte <='f')
    	return byte - 'a' + 10;
    else if(byte >= 'A' && byte <='F')
    	return byte - 'A' + 10;
    return 100;
}

int hexValue(char *byte) {
	return 16*hexValue(byte[0]) + hexValue(byte[1]);
}

skColor hexColor(std::string hex) {
	if(hex=="")
		return skColor();

	char *s = hex.data();
	if(hex[0]=='#')
		s++;

	//Check for alpha
	if(hex.size() > 7)
		return skColor(hexValue(s), hexValue(s+2), hexValue(s+4), hexValue(s+6));
	return skColor(hexValue(s), hexValue(s+2), hexValue(s+4));
}