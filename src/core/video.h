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

	const int Flags[] {
		sf::Style::Close,
		sf::Style::Fullscreen,
		sf::Style::None,
		0
	};
}

namespace TargetMode {
	
	enum {
		None,
		Texture,
		Window,
		MainWindow
	};
}

class Renderer {
	int targetMode;
	sf::RenderTarget* target;

	bool IMGUI_UPDATE = false;
	sf::Clock clock;

	std::string title;
	Vector2 size;
	int displayMode;
	bool vsync;
	bool signalReload;

public:

	Rectangle camera;

	Renderer(int mode);
	~Renderer();

	const Vector2& getSize();
	void setSize(Vector2 _size);

	const std::string& getTitle();
	void setTitle(std::string _title);

	const int& getDisplayMode();
	void setDisplayMode(int _displayMode);

	const bool& getVSync();
	void setVSync(bool _vsync);

	bool reload();
	void close();
	void clear(sf::Color = sf::Color(0, 0, 0));
	void display();

	bool isOpen();
	const sf::Texture* getTexture();

	sf::RenderTexture* getRenderTexturePtr();
	sf::RenderWindow* getRenderWindowPtr();

    void draw(const sf::Drawable& drawable, const sf::RenderStates& states = sf::RenderStates::Default);
    void draw(const sf::Vertex* vertices, std::size_t vertexCount, sf::PrimitiveType type, const sf::RenderStates& states = sf::RenderStates::Default);
    void draw(const sf::VertexBuffer& vertexBuffer, const sf::RenderStates& states = sf::RenderStates::Default);
    void draw(const sf::VertexBuffer& vertexBuffer, std::size_t firstVertex, std::size_t vertexCount, const sf::RenderStates& states = sf::RenderStates::Default);

    void setView(const sf::View& view);
    sf::View getView();
    sf::View getDefaultView();

	Vector2 toScreen(const Vector2& vec);
	Rectangle toScreen(const Rectangle& rect);	
	Vector2 toReal(const Vector2& vec);
	Rectangle toReal(const Rectangle& rect);

	Vector2 getCameraScale();
};

namespace g {
	extern Renderer video;
};

#endif