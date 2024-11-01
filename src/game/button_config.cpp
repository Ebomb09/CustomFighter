#include "button_config.h"

#include "core/input_interpreter.h"
#include "core/video.h"
#include "core/save.h"
#include "core/menu.h"

#include <stack>
#include <string>
#include <vector>
#include <iostream>

using std::vector, std::stack;

void ButtonConfig::run(Rectangle area) {

	// Save the previous draw frame
	sf::Texture prev;
	if(g::video.getRenderWindowPtr() && prev.create(g::video.getSize().x, g::video.getSize().y)) {
		prev.update(*g::video.getRenderWindowPtr());
	}

	enum {
		SelectMode,
		ControllerMode,
		InputMode,
		Back,
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

		for(int i = 0; i < MAX_PLAYERS; i ++) {

			Rectangle subArea = {
				area.x + area.w / 2 * (float)(i % 2) + 2, 
				area.y + area.h / 2 * (float)(i / 2) + 2, 
				area.w / 2 - 4, 
				area.h / 2 - 4
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
					options.push_back({InputMode, i, Button::Notation[i], "fight"});
					options.push_back({InputMode, i, g::input.buttonName(b.index, b.button[i])});			
				}

				options.push_back({});
				options.push_back({});

				options.push_back({Back, "Back"});
				options.push_back({Back, ""});

				int res = Menu::Table(options, 2, true, &hover[i].top(), i, subArea);

				if(res == Menu::Accept) {

					if(options[hover[i].top()].id == Back) {
						return;

					}else if(options[hover[i].top()].id == ControllerMode) {
						step[i] = ControllerMode;
						hover[i].push(0);

					}else if(options[hover[i].top()].id == InputMode) {
						step[i] = InputMode;

						hover[i].push(options[hover[i].top()].data);	// Button
						hover[i].push(0);								// Input
					}

				}else if(res == Menu::Decline) {
					return;
				}

			}else if(step[i] == InputMode) {

				if(Menu::WaitForInput(&hover[i].top(), i, subArea) == Menu::Accept) {
					int input = hover[i].top();
					hover[i].pop();

					int button = hover[i].top();
					hover[i].pop();

					b.button[button] = input;
					g::save.saveButtonConfig(i, b);

					step[i] = SelectMode;
				}

			}else if(step[i] == ControllerMode) {

				if(Menu::WaitForController(&hover[i].top(), i, subArea) == Menu::Accept) {
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
