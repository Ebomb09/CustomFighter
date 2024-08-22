#ifndef GAME_RENDERER_INSTANCE_H
#define GAME_RENDERER_INSTANCE_H

#include <string>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <nfd.h>

#include "math.h"
#include "player.h"

class RenderInstance : public sf::RenderWindow {
	bool IMGUI_UPDATE = false;

public:
	sf::Clock clock;

	Camera camera;

	RenderInstance();
	~RenderInstance();

	void clear();
	void display();

	bool init(unsigned int w, unsigned int h, std::string title);
};

namespace g {
	extern RenderInstance video;
};

#endif