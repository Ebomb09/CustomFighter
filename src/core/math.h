#ifndef GAME_MATH_H
#define GAME_MATH_H

#include <SFML/Graphics.hpp>

#define PI 3.14159265359

struct Vector2 {
	float x, y;

	Vector2(float _x=0.f, float _y=0.f);
	Vector2(sf::Vector2f);
	Vector2(sf::Vector2i);
	Vector2(sf::Vector2u);
	Vector2(sf::Vertex);

	operator sf::Vector2f();
	operator sf::Vector2i();
	operator sf::Vector2u();		
	operator sf::Vertex();	

	Vector2 operator-(const Vector2& v2);
	Vector2 operator+(const Vector2& v2);	
	Vector2 operator*(const float n);
	Vector2 operator/(const float n);
	Vector2 operator*(const Vector2& v2);
	Vector2 operator/(const Vector2& v2);
	Vector2& operator-=(const Vector2& v2);
	Vector2& operator+=(const Vector2& v2);
	Vector2& operator*=(const float n);
	Vector2& operator/=(const float n);
	Vector2& operator*=(const Vector2& v2);
	Vector2& operator/=(const Vector2& v2);
	bool operator==(const Vector2& v2);
	bool operator!=(const Vector2& v2);

	float getAngle();
	float getDistance();
	Vector2 translate(float rad, float h);
	Vector2 rotate(float angle, Vector2 origin = {0,0});
};

struct Circle {
	float x, y, radius;

	operator sf::CircleShape();
};

struct Rectangle {
	float x, y, w, h;

	Rectangle getRatio(float width, float height);

	operator sf::RectangleShape();
	operator sf::FloatRect();
};

struct HurtBox : Rectangle {
	int armour = 0;
};

namespace HitType {

	enum {
		High,
		Mid,
		Low,
		Unblockable,
		Total
	};

	const std::string String[] {
		"High",
		"Mid",
		"Low",
		"Unblockable",
		"Total"
	};
};

struct HitBox : Rectangle {
	Vector2 force;
	int type = HitType::Mid;
	int damage = 0;
	int hitStun = 0;
	int blockStun = 0;
	bool knockdown = false;
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