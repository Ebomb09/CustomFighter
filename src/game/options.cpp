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
		sh.setFillColor(sf::Color::Black);
		g::video.draw(sh);

		// Draw the options
		vector<Menu::Option> options;					

		options.push_back({VideoSettings, "Video Settings"});	
		options.push_back({ControllerSettings, "Controller Settings"});	
		options.push_back({});		
		options.push_back({Back, "Back"});

		int res = Menu::Table(options, 1, true, &hover, 0, area);

		if(res == Menu::Accept) {

			if(options[hover].id == ControllerSettings) {
				ButtonConfig::run({area.x + 128, area.y, area.w + 256, area.h});

			}else if(options[hover].id == VideoSettings) {
				VideoConfig::run({area.x + 128, area.y, area.w, area.h});

			}else if(options[hover].id == Back) {
				return;
			}
			
		}else if(res == Menu::Decline) {
			return;
		}
		
		g::video.display();
	}	
}