#include "Vector.h"

Vector2f operator*(const Vector2f &first, const Vector2i &second) {
	return Vector2f(first.x * second.x, first.y * second.y);
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