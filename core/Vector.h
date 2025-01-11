#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <iostream>
#include <math.h>

#define RT2O2 sqrt(2) / 2.0

//Essential vector functions
sf::Vector2f vectorLength(sf::Vector2f dir, double distance);
float distance(sf::Vector2f start, sf::Vector2f end=sf::Vector2f(0,0));
sf::Vector2f operator*(const sf::Vector2f &first, const sf::Vector2f &second);
sf::Vector2f operator*(const sf::Vector2f &first, const sf::Vector2i &second);
sf::Vector2i operator*(const sf::Vector2i &first, const sf::Vector2i &second);
sf::Vector2f operator/(const sf::Vector2f &first, const sf::Vector2f &second);
std::ostream& operator<<(std::ostream& os, const sf::Vector2f &pos);
std::ostream& operator<<(std::ostream& os, const sf::Vector2i &pos);