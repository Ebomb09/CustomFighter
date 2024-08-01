#ifndef GAME_MATH_H
#define GAME_MATH_H

#include <SFML/Graphics.hpp>

#define PI 3.14159265359

struct Vector2 {
	float x, y;

	Vector2(float _x=0.f, float _y=0.f);
	Vector2(sf::Vector2f);
	Vector2(sf::Vertex);

	operator sf::Vector2f();
	operator sf::Vertex();	

	Vector2 operator-(const Vector2& v2);
	Vector2 operator+(const Vector2& v2);	
	Vector2 operator*(const float n);
	Vector2 operator/(const float n);
	Vector2 operator*(const Vector2 v2);
	Vector2 operator/(const Vector2 v2);
	Vector2& operator-=(const Vector2& v2);
	Vector2& operator+=(const Vector2& v2);
	Vector2& operator*=(const float n);
	Vector2& operator/=(const float n);

	float getAngle();
	float getDistance();
	Vector2 translate(float rad, float h);
};

struct Circle {
	float x, y, radius;

	operator sf::CircleShape();
};

struct Rectangle {
	float x, y, w, h; 

	operator sf::RectangleShape();
};

struct HurtBox : Rectangle {
	int armour = 0;
};

struct HitBox : Rectangle {
	Vector2 impulse;
	int damage = 0;
	int hitStun = 0;
	int blockStun = 0;
	bool knockdown = false;
};

struct Camera {
	float x, y, w, h, screen_w, screen_h;

	Vector2 getScreenScale();

	Vector2 getReal(Vector2 pos);
	Rectangle getReal(Rectangle rect);

	Vector2 getScreen(Vector2 pos);
	Rectangle getScreen(Rectangle rect);
};

namespace Real {
	bool pointInCircle(const Vector2& point, const Circle& circle);
	bool pointInRectangle(const Vector2& point, const Rectangle& rectangle);
	bool rectangleInRectangle(const Rectangle& r1, const Rectangle& r2);
};

namespace Screen {
	using Real::pointInCircle;
	bool pointInRectangle(const Vector2& point, const Rectangle& rectangle);
	bool rectangleInRectangle(const Rectangle& r1, const Rectangle& r2);	
};

#endif