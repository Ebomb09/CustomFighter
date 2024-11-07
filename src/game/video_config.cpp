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

		// Create the options menu
		Menu::Config conf;
		conf.draw_Area = area;

		conf.data_Columns = 2;
		conf.data_GroupByRow = true;

		conf.push_back({Resolution, "Resolution"});
		conf.push_back({Resolution, std::to_string((int)g::save.resolution.x) + "x" + std::to_string((int)g::save.resolution.y)});

		conf.push_back({DisplayMode, "Display Mode"});
		conf.push_back({DisplayMode, DisplayMode::String[g::save.displayMode]});	

		conf.push_back({VSync, "V-Sync"});	
		conf.push_back({VSync, (g::save.vsync) ? " On" : "Off"});

		conf.push_back({Apply, "Apply"});
		conf.push_back({});

		conf.push_back({});
		conf.push_back({});

		conf.push_back({Back, "Back"});
		conf.push_back({});

		int res = Menu::Table(conf, 0, &hover, true);

		if(res == Menu::Accept) {

			if(conf[hover].id == Back) {
				return;

			}else if(conf[hover].id == Apply) {
				g::save.saveVideoConfig();
				g::video.setSize(g::save.resolution);
				g::video.setDisplayMode(g::save.displayMode);
				g::video.setVSync(g::save.vsync);				
				g::video.reload();
			}

		}else if(res == Menu::Decline) {
			return;

		}else {

			if(conf[hover].id == Resolution) {
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

			}else if(conf[hover].id == DisplayMode) {
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

			}else if(conf[hover].id == VSync) {

				if(g::input.pressed(b.index, b.Right) || g::input.pressed(b.index, b.Left)) 
					g::save.vsync = !g::save.vsync;
			}
		}
	    g::video.display();
	}	
}