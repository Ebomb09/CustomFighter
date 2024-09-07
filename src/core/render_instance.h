#ifndef GAME_RENDERER_INSTANCE_H
#define GAME_RENDERER_INSTANCE_H

#include "math.h"

#include <string>
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui-SFML.h>
#include <nfd.h>

namespace DisplayMode {
	
	enum {
		Window,
		Fullscreen,
		Borderless,
		Total
	};

	static const std::string String[] {
		"Window",
		"Fullscreen",
		"Borderless",
		"Total"
	};
}


class RenderInstance : public sf::RenderWindow {
	bool IMGUI_UPDATE = false;

public:
	sf::Clock clock;
	std::string title;
	Camera camera;

	~RenderInstance();

	void clear();
	void display();

	bool init(std::string title);
	void reload();
};

namespace g {
	extern RenderInstance video;
};

#endif