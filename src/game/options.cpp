#include "options.h"
#include "button_config.h"
#include "video_config.h"

#include "core/menu.h"
#include "core/input_interpreter.h"
#include "core/video.h"

#include <vector>
#include <string>

using std::string, std::vector;

void Options::run() {

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

		Rectangle area = {0, 0, (float)g::video.getSize().x, (float)g::video.getSize().y};	
		vector<Menu::Option> options;					

		options.push_back({VideoSettings, "Video Settings"});	
		options.push_back({ControllerSettings, "Controller Settings"});	
		options.push_back({});		
		options.push_back({Back, "Back"});

		int res = Menu::Table(options, 1, true, &hover, 0, area);

		if(res == Menu::Accept) {

			if(options[hover].id == ControllerSettings) {
				ButtonConfig::run();

			}else if(options[hover].id == VideoSettings) {
				VideoConfig::run();

			}else if(options[hover].id == Back) {
				return;
			}
			
		}else if(res == Menu::Decline) {
			return;
		}
		
		g::video.display();
	}	
}