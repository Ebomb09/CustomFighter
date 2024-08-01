#ifndef GAME_INPUT_INTERPRETER_H
#define GAME_INPUT_INTERPRETER_H

#include <SFML/Graphics.hpp>
#include "math.h"

struct InputInterpreter {

	// Keyboard
	bool keyPressed[sf::Keyboard::Key::KeyCount];
	bool keyHeld[sf::Keyboard::Key::KeyCount];
	bool keyReleased[sf::Keyboard::Key::KeyCount];	

	// Mouse
	Vector2 mouseLastPosition;
	Vector2 mousePosition;
	Vector2 mouseMove;
	float mouseScroll;
	bool mousePressed[sf::Mouse::Button::ButtonCount];
	bool mouseHeld[sf::Mouse::Button::ButtonCount];
	bool mouseReleased[sf::Mouse::Button::ButtonCount];

	bool windowClose;

	InputInterpreter();

	void prepEvents();
	void processEvent(const sf::Event& event);
};

namespace g {
	extern InputInterpreter input;
};

#endif