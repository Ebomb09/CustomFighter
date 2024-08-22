#include "render_instance.h"
#include "save.h"

RenderInstance g::video = RenderInstance();

using std::vector, std::string;

RenderInstance::RenderInstance() {

}

bool RenderInstance::init(unsigned int w, unsigned int h, string title) {
	create(sf::VideoMode(w, h), title);
	NFD_Init();
	ImGui::SFML::Init(*this);
	camera = {0, 0, (float)w, (float)h, (float)w, (float)h};
	setFramerateLimit(60);
	setKeyRepeatEnabled(false);
	return true;
}

RenderInstance::~RenderInstance() {
	NFD_Quit();
	close();
	ImGui::SFML::Shutdown();
}

void RenderInstance::clear() {

	// Update ImGui, only once
	if(!IMGUI_UPDATE) {
	    ImGui::SFML::Update(*this, g::video.clock.restart());
	    IMGUI_UPDATE = true;
	}

	// Clear buffer
	sf::RenderWindow::clear();
}

void RenderInstance::display() {

	// Render ImGui last, only once
	if(IMGUI_UPDATE) {
		ImGui::SFML::Render(*this);
		IMGUI_UPDATE = false;
	}

	// Swap buffer
	sf::RenderWindow::display();
}