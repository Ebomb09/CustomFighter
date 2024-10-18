#include "math.h"

#include <cmath>

Vector2::Vector2(float _x, float _y) {
	x = _x;
	y = _y;
}

Vector2::Vector2(sf::Vector2f v) { x = v.x; y = v.y; }
Vector2::Vector2(sf::Vector2i v) { x = v.x; y = v.y; }
Vector2::Vector2(sf::Vector2u v) { x = v.x; y = v.y; }
Vector2::Vector2(sf::Vertex v) { x = v.position.x; y = v.position.y; }

Vector2::operator sf::Vector2f() { return sf::Vector2f(x, y); }
Vector2::operator sf::Vector2i() { return sf::Vector2i(x, y); }
Vector2::operator sf::Vector2u() { return sf::Vector2u(x, y); }
Vector2::operator sf::Vertex() { return sf::Vertex(sf::Vector2f(x, y)); }

Vector2 Vector2::operator+(const Vector2& v2) {
	return {x + v2.x, y + v2.y};
}

Vector2 Vector2::operator-(const Vector2& v2) {
	return {x - v2.x, y - v2.y};
}

Vector2 Vector2::operator*(const float n) {
	return {x * n, y * n};
}

Vector2 Vector2::operator/(const float n) {
	return {x / n, y / n};
}

Vector2 Vector2::operator*(const Vector2& v2) {
	return {x * v2.x, y * v2.y};
}

Vector2 Vector2::operator/(const Vector2& v2) {
	return {x / v2.x, y / v2.y};
}

Vector2& Vector2::operator+=(const Vector2& v2) {
	x += v2.x;
	y += v2.y;
	return *this;
}

Vector2& Vector2::operator-=(const Vector2& v2) {
	x -= v2.x;
	y -= v2.y;
	return *this;
}

Vector2& Vector2::operator*=(const float n) {
	x *= n;
	y *= n;
	return *this;	
}

Vector2& Vector2::operator/=(const float n) {
	x /= n;
	y /= n;
	return *this;	
}

Vector2& Vector2::operator*=(const Vector2& v2) {
	x *= v2.x;
	y *= v2.y;
	return *this;	
}

Vector2& Vector2::operator/=(const Vector2& v2) {
	x /= v2.x;
	y /= v2.y;
	return *this;	
}

bool Vector2::operator==(const Vector2& v2) {
	return (x == v2.x && y == v2.y);
}

bool Vector2::operator!=(const Vector2& v2) {
	return (x != v2.x || y != v2.y);
}

float Vector2::getAngle() {
	return std::atan2(y, x);
}

float Vector2::getDistance() {
	return std::sqrt(std::pow(x, 2) + std::pow(y, 2));
}

Vector2 Vector2::translate(float rad, float h) {
	return {x + std::cos(rad) * h, y + std::sin(rad) * h};
}

Vector2 Vector2::rotate(float angle, Vector2 origin) {
	
	return origin + Vector2(
		(x - origin.x) * std::cos(angle) - (y - origin.y) * std::sin(angle), 
		(y - origin.y) * std::cos(angle) + (x - origin.x) * std::sin(angle)		
	);
}

bool Real::pointInCircle(const Vector2& point, const Circle& circle) {
	float a = std::pow(point.x - circle.x, 2);
	float b = std::pow(point.y - circle.y, 2);
	float c = std::sqrt(a + b);

	return (c <= circle.radius);
}

bool Real::pointInRectangle(const Vector2& point, const Rectangle& rect) {
	return 	(point.x >= rect.x && point.x <= rect.x + rect.w) &&
			(point.y <= rect.y && point.y >= rect.y - rect.h);
}

bool Real::rectangleInRectangle(const Rectangle& r1, const Rectangle& r2) {
	return 	(r1.x + r1.w >= r2.x && r1.x <= r2.x + r2.w) &&
			(r1.y - r1.h <= r2.y && r1.y >= r2.y - r2.h);
}

bool Screen::pointInRectangle(const Vector2& point, const Rectangle& rect) {
	return 	(point.x >= rect.x && point.x <= rect.x + rect.w) &&
			(point.y >= rect.y && point.y <= rect.y + rect.h);
}

bool Screen::rectangleInRectangle(const Rectangle& r1, const Rectangle& r2) {
	return 	(r1.x + r1.w >= r2.x && r1.x <= r2.x + r2.w) &&
			(r1.y + r1.h >= r2.y && r1.y <= r2.y + r2.h);
}

Circle::operator sf::CircleShape() {
	sf::CircleShape sh;
	sh.setPosition({x, y});
	sh.setRadius(radius);
	return sh;
}

Rectangle::operator sf::RectangleShape() {
	sf::RectangleShape sh;
	sh.setPosition({x, y});
	sh.setSize({w, h});
	return sh;
}

Rectangle::operator sf::FloatRect() {
	return sf::FloatRect(x, y, w, h);
}

Rectangle::operator sf::VertexArray() {
	sf::VertexArray ar;
	ar.append(Vector2(x, y));
	ar.append(Vector2(x + w, y));
	ar.append(Vector2(x + w, y + h));
	ar.append(Vector2(x, y + h));
	return ar;
}

Rectangle Rectangle::getRatio(float width, float height) {

	return {
		x / width, 
		y / height,
		w / width, 
		h / height		
	};
}