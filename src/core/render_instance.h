#ifndef GAME_RENDERER_INSTANCE_H
#define GAME_RENDERER_INSTANCE_H

#include <string>
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <nfd.h>

#include "math.h"

struct RenderInstance {
	sf::RenderWindow window;
	sf::Clock clock;

	Camera camera;

	RenderInstance();
	~RenderInstance();

	bool init(unsigned int w, unsigned int h, std::string title);
};

namespace g {
	extern RenderInstance video;
};

#endif