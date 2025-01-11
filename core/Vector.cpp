#include "Vector.h"

//Create a vector with fixed length in any direction
sf::Vector2f vectorLength(sf::Vector2f dir, double distance) {
	float xOffset = 0;
	float yOffset = 0;
	if(dir.x == 0 && dir.y == 0)
		return sf::Vector2f(0, 0);
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

	return sf::Vector2f(xOffset, yOffset);
}

//Get length of a vector or distance between points
float distance(sf::Vector2f start, sf::Vector2f end) {
	return std::sqrt(std::pow(end.x - start.x, 2) + std::pow(end.y - start.y, 2));
}

sf::Vector2f operator*(const sf::Vector2f &first, const sf::Vector2f &second) {
	return sf::Vector2f(first.x * second.x, first.y * second.y);
}
sf::Vector2f operator*(const sf::Vector2f &first, const sf::Vector2i &second) {
	return sf::Vector2f(first.x * second.x, first.y * second.y);
}
sf::Vector2i operator*(const sf::Vector2i &first, const sf::Vector2i &second) {
	return sf::Vector2i(first.x * second.x, first.y * second.y);
}
sf::Vector2f operator/(const sf::Vector2f &first, const sf::Vector2f &second) {
	return sf::Vector2f(first.x / second.x, first.y / second.y);
}

std::ostream& operator<<(std::ostream& os, const sf::Vector2f &pos) {
	return os << "(" << std::to_string(pos.x) << "," << std::to_string(pos.y) << ") ";
}

std::ostream& operator<<(std::ostream& os, const sf::Vector2i &pos) {
	return os << "(" << std::to_string(pos.x) << "," << std::to_string(pos.y) << ") ";
}