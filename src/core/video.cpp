#include "video.h"
#include "save.h"

#include <cmath>
#include <iostream>

Renderer g::video = Renderer(TargetMode::MainWindow);

using std::string;

static bool initSystems(sf::RenderWindow* window) {

	if(NFD_Init() != NFD_OKAY) {
		std::cerr << "NFD failed to init\n";
		return false;
	}

	if(!ImGui::SFML::Init(*window)) {
		std::cerr << "ImGui::SFML failed to init\n";
		return false;
	}

	return true;
}

static void closeSystems() {
	NFD_Quit();
	ImGui::SFML::Shutdown();
}

Renderer::Renderer(int mode) {
	targetMode = mode;
	target = NULL;

	switch(targetMode) {

		case TargetMode::MainWindow:
		case TargetMode::Window: {
			target = new sf::RenderWindow;
			break;
		}

		case TargetMode::Texture: {
			target = new sf::RenderTexture;
			break;
		}
	}

	if(mode == TargetMode::MainWindow) 
		initSystems((sf::RenderWindow*)target);

	title = "";
	size = {0, 0};
	displayMode = 0;
	vsync = false;
	signalReload = false;
}

Renderer::~Renderer() {

	if(targetMode == TargetMode::MainWindow)
		closeSystems();

	if(target)
		delete target;
}

const Vector2& Renderer::getSize() {
	return size;
}

void Renderer::setSize(Vector2 _size) {
	_size.x = std::round(_size.x);
	_size.y = std::round(_size.y);

	if(size != _size && _size.x > 0 && _size.y > 0) {
		size = _size;
		signalReload = true;
	}
}

const std::string& Renderer::getTitle() {
	return title;
}

void Renderer::setTitle(std::string _title) {

	if(title != _title) {
		title = _title;
		signalReload = true;
	}
}

const int& Renderer::getDisplayMode() {
	return displayMode;
}

void Renderer::setDisplayMode(int _displayMode) {

	if(displayMode != _displayMode && _displayMode >= 0 && _displayMode < DisplayMode::Total) {
		displayMode = _displayMode;
		signalReload = true;
	}
}

const bool& Renderer::getVSync() {
	return vsync;
}

void Renderer::setVSync(bool _vsync) {

	if(vsync != _vsync) {
		vsync = _vsync;
		signalReload = true;
	}
}

bool Renderer::reload() {

	if(!target)
		return false;

	switch(targetMode) {

		case TargetMode::MainWindow:
		case TargetMode::Window: {

			if(signalReload) {
				sf::RenderWindow* win = getRenderWindowPtr();

				win->create(sf::VideoMode(size.x, size.y), title, DisplayMode::Flags[displayMode]);
				win->setVerticalSyncEnabled(vsync);
				win->setFramerateLimit(60);
				win->setKeyRepeatEnabled(false);

				signalReload = false;
			}
			break;
		}

		case TargetMode::Texture: {

			if(signalReload) {
				sf::RenderTexture* tex = getRenderTexturePtr();

				if(tex->create(size.x, size.y))
					signalReload = false;
			}
			break;
		}
	}

	// If still signaling then something failed
	if(signalReload) 
		return false;

	// All good
	camera = {0, 0, size.x, size.y};
	return true;
}

void Renderer::close() {

	if(target)
		delete target;

	target = NULL;
}

void Renderer::clear(sf::Color color) {

	if(!target)
		return;

	if(targetMode == TargetMode::MainWindow) {

		// Update ImGui, only once
		if(!IMGUI_UPDATE) {
		    ImGui::SFML::Update(*(sf::RenderWindow*)target, clock.restart());
		    IMGUI_UPDATE = true;
		}
	}

	target->clear(color);
}

void Renderer::display() {

	if(!target)
		return;

	switch(targetMode) {

		case TargetMode::Texture: {
			((sf::RenderTexture*)target)->display();
			break;
		}

		case TargetMode::MainWindow: {

			// Render ImGui last, only once
			if(IMGUI_UPDATE) {
				ImGui::SFML::Render(*target);
				IMGUI_UPDATE = false;
			}	
		}

		case TargetMode::Window: {
			((sf::RenderWindow*)target)->display();
			break;
		}
	}
}

bool Renderer::isOpen() {

	if(!target)
		return false;

	if(targetMode == TargetMode::Window || targetMode == TargetMode::MainWindow)
		return ((sf::RenderWindow*)target)->isOpen();

	return false;
}

const sf::Texture* Renderer::getTexture() {

	if(!target)
		return NULL;

	if(targetMode == TargetMode::Texture) 
		return &getRenderTexturePtr()->getTexture();

	return NULL;
}

sf::RenderTexture* Renderer::getRenderTexturePtr() {

	if(!target)
		return NULL;

	if(targetMode == TargetMode::Texture)
		return (sf::RenderTexture*)target;

	return NULL;
}

sf::RenderWindow* Renderer::getRenderWindowPtr() {

	if(!target)
		return NULL;
	
	if(targetMode == TargetMode::Window || targetMode == TargetMode::MainWindow)
		return (sf::RenderWindow*)target;
	return NULL;
}

void Renderer::draw(const sf::Drawable& drawable, const sf::RenderStates& states) {
	if(target) target->draw(drawable, states);
}

void Renderer::draw(const sf::Vertex* vertices, std::size_t vertexCount, sf::PrimitiveType type, const sf::RenderStates& states) {
	if(target) target->draw(vertices, vertexCount, type, states);
}

void Renderer::draw(const sf::VertexBuffer& vertexBuffer, const sf::RenderStates& states) {
	if(target) target->draw(vertexBuffer, states);
}

void Renderer::draw(const sf::VertexBuffer& vertexBuffer, std::size_t firstVertex, std::size_t vertexCount, const sf::RenderStates& states) {
	if(target) target->draw(vertexBuffer, firstVertex, vertexCount, states);
}

void Renderer::setView(const sf::View& view) {
	if(target) target->setView(view);
}

sf::View Renderer::getView() {
	if(target) return target->getView();
	return {};
}

sf::View Renderer::getDefaultView() {
	if(target) return target->getDefaultView();
	return {};
}

Vector2 Renderer::toScreen(const Vector2& vec) {
	Vector2 percent = {
		(vec.x - camera.x) / camera.w, 
		(camera.y - vec.y) / camera.h
	};
	return {percent.x * size.x, percent.y * size.y};
}

Rectangle Renderer::toScreen(const Rectangle& rect) {
	Vector2 pos[] = {
		toScreen(Vector2(rect.x, rect.y)),
		toScreen(Vector2(rect.x + rect.w, rect.y - rect.h))
	};
	return {pos[0].x, pos[0].y, pos[1].x - pos[0].x, pos[1].y - pos[0].y};
}

Vector2 Renderer::toReal(const Vector2& vec) {
	Vector2 percent = {
		vec.x / size.x, 
		vec.y / size.y
	};
	return {camera.x + percent.x * camera.w, camera.y - percent.y * camera.h};
}

Rectangle Renderer::toReal(const Rectangle& rect) { 
	Vector2 pos[] = {
		toReal(Vector2(rect.x, rect.y)),
		toReal(Vector2(rect.x + rect.w, rect.y + rect.h))
	};
	return {pos[0].x, pos[0].y, pos[1].x - pos[0].x, pos[0].y - pos[1].y};
}

Vector2 Renderer::getCameraScale() {
	return {
		camera.w / size.x,
		camera.h / size.y
	};
}