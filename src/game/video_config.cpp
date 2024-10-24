#include "video_config.h"

#include "core/menu.h"
#include "core/input_interpreter.h"
#include "core/video.h"
#include "core/save.h"

#include <vector>
#include <string>

using std::string, std::vector;

void VideoConfig::run(Rectangle area) {

	// Save the previous draw frame
	sf::Texture prev;
	if(g::video.getRenderWindowPtr() && prev.create(g::video.getSize().x, g::video.getSize().y)) {
		prev.update(*g::video.getRenderWindowPtr());
	}

	enum {
		Resolution,
		DisplayMode,
		VSync,
		Apply,
		Disregard,
		Back
	};

	Button::Config b = g::save.getButtonConfig(0);
	int hover = 0;

	while (g::video.isOpen()) {
	    g::input.pollEvents();

	    g::video.clear();

		// Draw the previous frame
		sf::RectangleShape sh = Rectangle{0, 0, g::video.getSize().x, g::video.getSize().y};
		sh.setTexture(&prev);
		g::video.draw(sh);

		// Draw the menu area
		sh = area;
		sh.setFillColor(sf::Color(32, 32, 32));
		sh.setOutlineThickness(4);
		sh.setOutlineColor(sf::Color::White);
		g::video.draw(sh);

		// Draw the options
		vector<Menu::Option> options;

		options.push_back({Resolution, "Resolution"});
		options.push_back({Resolution, std::to_string((int)g::save.resolution.x) + "x" + std::to_string((int)g::save.resolution.y)});

		options.push_back({DisplayMode, "Display Mode"});
		options.push_back({DisplayMode, DisplayMode::String[g::save.displayMode]});	

		options.push_back({VSync, "V-Sync"});	
		options.push_back({VSync, (g::save.vsync) ? " On" : "Off"});

		options.push_back({Apply, "Apply"});
		options.push_back({});

		options.push_back({});
		options.push_back({});

		options.push_back({Back, "Back"});
		options.push_back({});

		int res = Menu::Table(options, 2, true, &hover, 0, area);

		if(res == Menu::Accept) {

			if(options[hover].id == Back) {
				return;

			}else if(options[hover].id == Apply) {
				g::save.saveVideoConfig();
				g::video.setSize(g::save.resolution);
				g::video.setDisplayMode(g::save.displayMode);
				g::video.setVSync(g::save.vsync);				
				g::video.reload();
			}

		}else if(res == Menu::Decline) {
			return;

		}else {

			if(options[hover].id == Resolution) {
				int index = 0;

				for(int i = 0; i < sf::VideoMode::getFullscreenModes().size(); i ++) {

					if(g::save.resolution.x == sf::VideoMode::getFullscreenModes()[i].width &&
						g::save.resolution.y == sf::VideoMode::getFullscreenModes()[i].height) {

						index = i;
						break;
					}
				}

				if(g::input.pressed(b.index, b.Right)) 
					index --;

				if(g::input.pressed(b.index, b.Left))
					index ++;

				if(index < 0)
					index = sf::VideoMode::getFullscreenModes().size() - 1;

				else if(index >= sf::VideoMode::getFullscreenModes().size())
					index = 0;

				g::save.resolution.x = sf::VideoMode::getFullscreenModes()[index].width;
				g::save.resolution.y = sf::VideoMode::getFullscreenModes()[index].height;

			}else if(options[hover].id == DisplayMode) {
				int index = g::save.displayMode;

				if(g::input.pressed(b.index, b.Right)) 
					index ++;

				if(g::input.pressed(b.index, b.Left))
					index --;

				if(index < 0)
					index = DisplayMode::Total - 1;

				else if(index >= DisplayMode::Total)
					index = 0;

				g::save.displayMode = index;

			}else if(options[hover].id == VSync) {

				if(g::input.pressed(b.index, b.Right) || g::input.pressed(b.index, b.Left)) 
					g::save.vsync = !g::save.vsync;
			}
		}
	    g::video.display();
	}	
}