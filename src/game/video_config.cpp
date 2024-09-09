#include "video_config.h"

#include "core/menu.h"
#include "core/input_interpreter.h"
#include "core/render_instance.h"
#include "core/save.h"

#include <iostream>
#include <vector>
#include <string>

using std::string, std::vector;

void VideoConfig::run() {

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

		Rectangle area = {0, 0, (float)g::video.getSize().x, (float)g::video.getSize().y};
		vector<Menu::Option> options;

		options.push_back({"Resolution", Resolution});
		options.push_back({std::to_string(g::save.resolutionWidth) + "x" + std::to_string(g::save.resolutionHeight), Resolution});

		options.push_back({"Display Mode", DisplayMode});
		options.push_back({DisplayMode::String[g::save.displayMode], DisplayMode});	

		options.push_back({"V-Sync", VSync});	
		options.push_back({(g::save.vsync) ? " On" : "Off", VSync});

		options.push_back({"Apply", Apply});
		options.push_back({"", Apply});

		options.push_back({"", Disregard});
		options.push_back({"", Disregard});

		options.push_back({"Back", Back});
		options.push_back({"", Back});

		int res = Menu::Table(options, 2, true, &hover, 0, area);

		if(res == Menu::Accept) {

			if(options[hover].id == Back) {
				return;

			}else if(options[hover].id == Apply) {
				g::save.saveVideoConfig();
				g::video.reload();
			}

		}else if(res == Menu::Decline) {
			return;

		}else {

			if(options[hover].id == Resolution) {
				int index = 0;

				for(int i = 0; i < sf::VideoMode::getFullscreenModes().size(); i ++) {

					if(g::save.resolutionWidth == sf::VideoMode::getFullscreenModes()[i].width &&
						g::save.resolutionHeight == sf::VideoMode::getFullscreenModes()[i].height) {

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

				g::save.resolutionWidth = sf::VideoMode::getFullscreenModes()[index].width;
				g::save.resolutionHeight = sf::VideoMode::getFullscreenModes()[index].height;

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