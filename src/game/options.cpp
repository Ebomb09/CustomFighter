#include "options.h"
#include "button_config.h"
#include "video_config.h"

#include "core/menu.h"
#include "core/input_interpreter.h"
#include "core/video.h"

#include <vector>
#include <string>

using std::string, std::vector;

void Options::run(Rectangle area) {

	// Save the previous draw frame
	sf::Texture prev;
	if(g::video.getRenderWindowPtr() && prev.create(g::video.getSize().x, g::video.getSize().y)) {
		prev.update(*g::video.getRenderWindowPtr());
	}

	enum {
		VideoSettings,
		ControllerSettings,
		Disregard,
		Back
	};

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

		conf.push_back({VideoSettings, "Video Settings"});	
		conf.push_back({ControllerSettings, "Controller Settings"});	
		conf.push_back({});		
		conf.push_back({Back, "Back"});

		int res = Menu::Table(conf, 0, &hover, true);

		if(res == Menu::Accept) {

			if(conf[hover].id == ControllerSettings) {
				ButtonConfig::run({area.x + 128, area.y, area.w + 256, area.h});

			}else if(conf[hover].id == VideoSettings) {
				VideoConfig::run({area.x + 128, area.y, area.w, area.h});

			}else if(conf[hover].id == Back) {
				return;
			}
			
		}else if(res == Menu::Decline) {
			return;
		}
		
		g::video.display();
	}	
}