#include "render_instance.h"

RenderInstance g::video = RenderInstance();

RenderInstance::RenderInstance() {

}

bool RenderInstance::init(unsigned int w, unsigned int h, std::string title) {
	window.create(sf::VideoMode(w, h), title);
	NFD_Init();
	ImGui::SFML::Init(window);
	camera = {0, 0, (float)w, (float)h, (float)w, (float)h};
	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);
	return true;
}

RenderInstance::~RenderInstance() {
	NFD_Quit();
	ImGui::SFML::Shutdown();
}