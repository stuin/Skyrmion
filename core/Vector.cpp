#include "UpdateList.h"

Vector2f operator*(const Vector2f &first, const Vector2i &second) {
	return Vector2f(first.x * second.x, first.y * second.y);
}
Vector2f operator*(const Vector2f &first, const float second) {
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

bool operator==(const TextureRect &first, const TextureRect &second) {
	return first.px == second.px && first.py == second.py && first.tx == second.tx && first.ty == second.ty;
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

//Convert screen space coordinates to global
Vector2f screenToGlobal(float x, float y) {
	Vector2f pos = Vector2f(x,y);
	if(pos.x < 0)
		pos.x += UpdateList::getScreenRect().getSize().x;
	if(pos.y < 0)
		pos.y += UpdateList::getScreenRect().getSize().y;
	pos *= UpdateList::getScaleFactor();
	pos += UpdateList::getCameraRect().getPosition();
	return pos;
}

//Event operators
bool operator==(const Event &first, const Event &second) {
	return first.type == second.type && first.down == second.down && first.code == second.code &&
		first.x == second.x && first.y == second.y;
}
bool operator!=(const Event &first, const Event &second) {
	return !(first == second);
}

//skColor operators
bool operator==(const skColor &first, const skColor &second) {
	return first.red == second.red && first.green == second.green && first.blue == second.blue && first.alpha == second.alpha;
}
bool operator!=(const skColor &first, const skColor &second) {
	return !(first == second);
}