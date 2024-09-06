#include "options.h"
#include "button_config.h"

#include "core/menu.h"
#include "core/input_interpreter.h"
#include "core/render_instance.h"
#include "core/save.h"

#include <stack>
#include <vector>
#include <string>

using std::string, std::vector, std::stack;

void Options::run() {

	enum {
		Main = -1,
		ControllerSettings = -2,
		Disregard = -3,
		Back = -4
	};

	int step = Main;
	std::stack<int> hover({0});

    while (g::video.isOpen()) {
        g::input.pollEvents();

        g::video.clear();

		if(step == Main) {
			Rectangle area = {0, 0, (float)g::video.getSize().x, (float)g::video.getSize().y};	
			vector<Menu::Option> options;					
			options.push_back({"Controller Settings", ControllerSettings});
			options.push_back({"", Disregard});		
			options.push_back({"Back", Back});

			int res = Menu::Table(options, 1, true, &hover.top(), 0, area);

			if(res == Menu::Accept) {

				if(options[hover.top()].id == ControllerSettings) {
					ButtonConfig::run();

				}else if(options[hover.top()].id == Back) {
					return;

				}else {

				}
				
			}else if(res == Menu::Decline) {
				return;
			}
		}
        g::video.display();
    }	
}