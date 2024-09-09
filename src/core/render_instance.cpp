#include "render_instance.h"
#include "save.h"

RenderInstance g::video = RenderInstance();

using std::string;

bool RenderInstance::init(string _title) {
	title = _title;
	reload();

	// Initialize subsystems
	if(NFD_Init() != NFD_OKAY)
		return false;

	if(ImGui::SFML::Init(*this))
		return false;

	return true;
}

void RenderInstance::reload() {

	// Close any previous windows
	if(isOpen())
		close();

	float width = g::save.resolutionWidth;
	float height = g::save.resolutionHeight;
	int flags = 0;

	if(g::save.displayMode == DisplayMode::Window) {
		flags = sf::Style::Close;

	}else if(g::save.displayMode == DisplayMode::Borderless) {
		flags = sf::Style::None;

	}else {
		flags = sf::Style::Fullscreen;
	}

	create(sf::VideoMode(width, height), title, flags);
	camera = {0, 0, width, height};	

	setVerticalSyncEnabled(g::save.vsync);
	setFramerateLimit(60);
	setKeyRepeatEnabled(false);
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