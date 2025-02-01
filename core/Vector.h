#pragma once

#include <iostream>
#include <math.h>

#define RT2O2 sqrt(2) / 2.0
#define PIO2 1.570796

using uint = unsigned int;
using sint = long unsigned int;

struct TextureRect {
	float px, py;
	float pwidth, pheight;
	int tx, ty;
	int twidth, theight;
	int rotation;
};

template <typename T>
class skVector2 {
public:
	T x = 0;
	T y = 0;

	skVector2() {}

	skVector2(T _x, T _y) {
		x = _x;
		y = _y;
	}

	template <typename T2>
	skVector2(skVector2<T2> _v) {
		x = (T)_v.x;
		y = (T)_v.y;
	}
};
typedef skVector2<int> Vector2i;
typedef skVector2<float> Vector2f;

template <typename T>
class Rect {
public:
	T left = 0;
	T top = 0;
	T width = 0;
	T height = 0;

	Rect() {}

	Rect(T _left, T _top, T _width, T _height) {
		left = _left;
		top = _top;
		width = _width;
		height = _height;
	}

	skVector2<T> getPosition() {
		return skVector2<T>(left, top);
	}

	skVector2<T> getSize() {
		return skVector2<T>(width, height);
	}

	bool contains(const skVector2<T> point) const {
		return point.x >= left && point.x <= left+width &&
			point.y >= top && point.y <= top+height;
	}

	bool intersects(const Rect<T>& rect) const {
		return left <= rect.left+rect.width && left+width >= rect.left &&
			top <= rect.top+rect.height && top+height >= rect.top;
	}
};
typedef Rect<int> IntRect;
typedef Rect<float> FloatRect;

//Vector with Vector operators
template <typename T>
constexpr skVector2<T> operator+(const skVector2<T> &first, const skVector2<T> &second) {
	return skVector2<T>(first.x + second.x, first.y + second.y);
}
template <typename T>
constexpr skVector2<T> operator-(const skVector2<T> &first, const skVector2<T> &second) {
	return skVector2<T>(first.x - second.x, first.y - second.y);
}
template <typename T>
constexpr skVector2<T> operator*(const skVector2<T> &first, const skVector2<T> &second) {
	return skVector2<T>(first.x * second.x, first.y * second.y);
}
template <typename T>
constexpr skVector2<T> operator/(const skVector2<T> &first, const skVector2<T> &second) {
	return skVector2<T>(first.x / second.x, first.y / second.y);
}

template <typename T>
constexpr skVector2<T>& operator+=(skVector2<T> &first, const skVector2<T> &second) {
	first.x += second.x;
    first.y += second.y;
    return first;
}
template <typename T>
constexpr skVector2<T>& operator-=(skVector2<T> &first, const skVector2<T> &second) {
	first.x -= second.x;
    first.y -= second.y;
    return first;
}
template <typename T>
constexpr skVector2<T>& operator*=(skVector2<T> &first, const skVector2<T> &second) {
	first.x *= second.x;
    first.y *= second.y;
    return first;
}
template <typename T>
constexpr skVector2<T>& operator/=(skVector2<T> &first, const skVector2<T> &second) {
	first.x /= second.x;
    first.y /= second.y;
    return first;
}

template <typename T>
constexpr bool operator==(const skVector2<T> &first, const skVector2<T> &second) {
	return first.x==second.x && first.y==second.y;
}
template <typename T>
constexpr bool operator!=(const skVector2<T> &first, const skVector2<T> &second) {
	return first.x!=second.x || first.y!=second.y;
}

//Vector with number operators
template <typename T>
constexpr skVector2<T> operator+(const skVector2<T>& first, T second) {
	return skVector2<T>(first.x + second, first.y + second);
}
template <typename T>
constexpr skVector2<T> operator-(const skVector2<T>& first, T second) {
	return skVector2<T>(first.x - second, first.y - second);
}
template <typename T>
constexpr skVector2<T> operator*(const skVector2<T>& first, T second) {
	return skVector2<T>(first.x * second, first.y * second);
}
template <typename T>
constexpr skVector2<T> operator/(const skVector2<T>& first, T second) {
	return skVector2<T>(first.x / second, first.y / second);
}

Vector2f operator*(const Vector2f &first, const Vector2i &second);
Vector2f operator*(const Vector2i &first, const float second);
Vector2f operator/(const Vector2f &first, const Vector2i &second);
std::ostream& operator<<(std::ostream& os, const Vector2f &pos);
std::ostream& operator<<(std::ostream& os, const Vector2i &pos);

bool operator==(const TextureRect &first, const TextureRect &second);

Vector2f vectorLength(Vector2f dir, double distance);
float distance(Vector2f start, Vector2f end=Vector2f(0,0));
Vector2i round(Vector2f pos);
Vector2f screenToGlobal(float x, float y);