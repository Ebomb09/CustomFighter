#include "button_config.h"

#include "core/input_interpreter.h"
#include "core/render_instance.h"
#include "core/save.h"
#include "core/menu.h"

#include <stack>
#include <string>
#include <vector>
#include <iostream>

using std::vector, std::stack;

void ButtonConfig::run() {

	enum {
		SelectMode = -1,
		ControllerMode = -2,
		InputMode = -3,
		Back = -4,
		Disregard = -5
	};

	int step[MAX_PLAYERS];
	stack<int> hover[MAX_PLAYERS];

	for(int i = 0; i < MAX_PLAYERS; i ++) {
		step[i] = SelectMode;
		hover[i].push(0);
	}

    while (g::video.isOpen()) {
        g::input.pollEvents();

        g::video.clear();

		for(int i = 0; i < MAX_PLAYERS; i ++) {

			Rectangle area = {
				(float)g::video.getSize().x / 2 * (float)(i % 2), 
				(float)g::video.getSize().y / 2 * (float)(i / 2), 
				(float)g::video.getSize().x / 2, 
				(float)g::video.getSize().y / 2
			};	

			Button::Config b = g::save.getButtonConfig(i);

			if(step[i] == SelectMode) {
				vector<Menu::Option> options;

				// Show controller name
				options.push_back({ControllerMode, "Controller"});
				options.push_back({ControllerMode, g::input.controllerName(b.index)});

				options.push_back({});
				options.push_back({});

				// Show all inputs
				for(int i = 0; i < Button::Total; i ++) {
					options.push_back({i, Button::String[i]});
					options.push_back({i, g::input.buttonName(b.index, b.button[i])});			
				}

				options.push_back({});
				options.push_back({});

				options.push_back({Back, "Back"});
				options.push_back({});

				int res = Menu::Table(options, 2, true, &hover[i].top(), i, area);

				if(res == Menu::Accept) {

					if(options[hover[i].top()].id == Back) {
						return;

					}else if(options[hover[i].top()].id == ControllerMode) {
						step[i] = ControllerMode;
						hover[i].push(0);

					}else {
						step[i] = InputMode;

						hover[i].push(options[hover[i].top()].id);	// Button
						hover[i].push(0);							// Input
					}

				}else if(res == Menu::Decline) {
					return;
				}

			}else if(step[i] == InputMode) {

				if(Menu::WaitForInput(&hover[i].top(), i, area) == Menu::Accept) {
					int input = hover[i].top();
					hover[i].pop();

					int button = hover[i].top();
					hover[i].pop();

					b.button[button] = input;
					g::save.saveButtonConfig(i, b);

					step[i] = SelectMode;
				}

			}else if(step[i] == ControllerMode) {

				if(Menu::WaitForController(&hover[i].top(), i, area) == Menu::Accept) {
					int index = hover[i].top();
					hover[i].pop();

					b.index = index;
					g::save.saveButtonConfig(i, b);

					step[i] = SelectMode;
				}				
			}
		}	
        g::video.display();
    }	
}
