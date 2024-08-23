#include <cmath>
#include "math.h"

Vector2::Vector2(float _x, float _y) {
	x = _x;
	y = _y;
}

Vector2::Vector2(sf::Vector2f v) {
	x = v.x;
	y = v.y;
}

Vector2::Vector2(sf::Vertex v) {
	x = v.position.x;
	y = v.position.y;
}

Vector2::operator sf::Vector2f() {
	return {x, y};
}

Vector2::operator sf::Vertex() {
	return sf::Vertex({x, y});
}

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

Vector2 Vector2::operator*(const Vector2 v2) {
	return {x * v2.x, y * v2.y};
}

Vector2 Vector2::operator/(const Vector2 v2) {
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

Vector2 Camera::getScreenScale() {
	return {w / screen_w, h / screen_h};
}

Vector2 Camera::getReal(Vector2 pos) {
	Vector2 percent = {pos.x / screen_w, pos.y / screen_h};
	return {x + percent.x * w, y - percent.y * h};
}

Rectangle Camera::getReal(Rectangle rect) { 
	Vector2 pos[] = {
		getReal(Vector2{rect.x, rect.y}),
		getReal(Vector2{rect.x + rect.w, rect.y + rect.h})
	};
	return {pos[0].x, pos[0].y, pos[1].x - pos[0].x, pos[0].y - pos[1].y};
}

Vector2 Camera::getScreen(Vector2 pos) {
	Vector2 percent = {(pos.x - x) / w, (y - pos.y) / h};
	return {percent.x * screen_w, percent.y * screen_h};
}

Rectangle Camera::getScreen(Rectangle rect) {
	Vector2 pos[] = {
		getScreen(Vector2{rect.x, rect.y}),
		getScreen(Vector2{rect.x + rect.w, rect.y - rect.h})
	};
	return {pos[0].x, pos[0].y, pos[1].x - pos[0].x, pos[1].y - pos[0].y};
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