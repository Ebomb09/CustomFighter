#ifndef GAME_INPUT_INTERPRETER_H
#define GAME_INPUT_INTERPRETER_H

#include <string>
#include <SFML/Graphics.hpp>
#include "math.h"

#define MAX_INPUTS 256
#define MAX_CONTROLLERS sf::Joystick::Count + 2
#define MOUSE_INDEX MAX_CONTROLLERS - 1
#define KEYBOARD_INDEX MAX_CONTROLLERS - 2

struct Controller {
	int lastInput = 0;

	bool pressed	[MAX_INPUTS];
	bool held		[MAX_INPUTS];
	bool released	[MAX_INPUTS];
};

struct InputInterpreter {
	Controller controller[MAX_CONTROLLERS];

	int lastController = 0;

	// Mouse
	Vector2 mouseLastPosition;
	Vector2 mousePosition;
	Vector2 mouseMove;
	float mouseScroll;

	void prepEvents();
	void pollEvents();
	void processEvent(const sf::Event& event);

	bool pressed(int index, int input);
	bool released(int index, int input);
	bool held(int index, int input);	

	std::string controllerName(int index);
	std::string buttonName(int index, int input);
};

namespace g {
	extern InputInterpreter input;
};

#endif