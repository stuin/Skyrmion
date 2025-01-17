#pragma once

#include <iostream>
#include <math.h>

#define RT2O2 sqrt(2) / 2.0
#define PIO2 1.570796

struct TextureRect {
	float px, py;
	int tx, ty;
	int width, height;
	float rotation;
};

template <typename T>
class Vector2 {
public:
	T x = 0;
	T y = 0;

	Vector2() {}

	Vector2(T _x, T _y) { 
		x = _x;
		y = _y;
	}
};
typedef Vector2<int> Vector2i;
typedef Vector2<float> Vector2f;

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

	bool contains(const Vector2<T> point) const {
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
constexpr Vector2<T> operator+(const Vector2<T> &first, const Vector2<T> &second) {
	return Vector2<T>(first.x + second.x, first.y + second.y);
}
template <typename T>
constexpr Vector2<T> operator-(const Vector2<T> &first, const Vector2<T> &second) {
	return Vector2<T>(first.x - second.x, first.y - second.y);
}
template <typename T>
constexpr Vector2<T> operator*(const Vector2<T> &first, const Vector2<T> &second) {
	return Vector2<T>(first.x * second.x, first.y * second.y);
}
template <typename T>
constexpr Vector2<T> operator/(const Vector2<T> &first, const Vector2<T> &second) {
	return Vector2<T>(first.x / second.x, first.y / second.y);
}

template <typename T>
constexpr Vector2<T>& operator+=(Vector2<T> &first, const Vector2<T> &second) {
	first.x += second.x;
    first.y += second.y;
    return first;
}
template <typename T>
constexpr Vector2<T>& operator-=(Vector2<T> &first, const Vector2<T> &second) {
	first.x -= second.x;
    first.y -= second.y;
    return first;
}
template <typename T>
constexpr Vector2<T>& operator*=(Vector2<T> &first, const Vector2<T> &second) {
	first.x *= second.x;
    first.y *= second.y;
    return first;
}
template <typename T>
constexpr Vector2<T>& operator/=(Vector2<T> &first, const Vector2<T> &second) {
	first.x /= second.x;
    first.y /= second.y;
    return first;
}

template <typename T>
constexpr bool operator==(const Vector2<T> &first, const Vector2<T> &second) {
	return first.x==second.x && first.y==second.y;
}
template <typename T>
constexpr bool operator!=(const Vector2<T> &first, const Vector2<T> &second) {
	return first.x!=second.x || first.y!=second.y;
}

//Vector with number operators
template <typename T>
constexpr Vector2<T> operator+(const Vector2<T>& first, T second) {
	return Vector2<T>(first.x + second, first.y + second);
}
template <typename T>
constexpr Vector2<T> operator-(const Vector2<T>& first, T second) {
	return Vector2<T>(first.x - second, first.y - second);
}
template <typename T>
constexpr Vector2<T> operator*(const Vector2<T>& first, T second) {
	return Vector2<T>(first.x * second, first.y * second);
}
template <typename T>
constexpr Vector2<T> operator/(const Vector2<T>& first, T second) {
	return Vector2<T>(first.x / second, first.y / second);
}

Vector2f operator*(const Vector2f &first, const Vector2i &second);
std::ostream& operator<<(std::ostream& os, const Vector2f &pos);
std::ostream& operator<<(std::ostream& os, const Vector2i &pos);

Vector2f vectorLength(Vector2f dir, double distance);
float distance(Vector2f start, Vector2f end);