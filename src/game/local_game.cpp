#include "local_game.h"
#include "game_state.h"
#include "options.h"

#include "core/input_interpreter.h"
#include "core/video.h"
#include "core/menu.h"
#include "core/audio.h"

using std::vector;

namespace ID {
    enum {
        Resume,
        Options,
        Exit
    };
};

bool LocalGame::run(vector<Player::Config> configs, int gameMode) {
	Game game;

    // Configure players
    for(int i = 0; i < configs.size(); i ++) {
        game.players[i].seatIndex = i;
        game.players[i].config = configs[i];
    }

    //game.players[1].seatIndex = -1;
    //game.players[1].aiLevel = 5;

    game.init(configs.size(), 3, 60, gameMode);

    bool pause = false;
    int hover = 0;

    while(g::video.isOpen()) {
        g::input.pollEvents();
        g::video.clear();

        if(g::input.pressed(KEYBOARD_INDEX, sf::Keyboard::Escape)) {
            pause = !pause;
        
            if(pause)
                g::audio.setVolume(25);
            else
                g::audio.setVolume(100);
        }

        if(!pause) {
            game.readInput();
            game.advanceFrame();
        }
        game.draw();

        if(pause) {
            vector<Menu::Option> options;
            options.push_back({ID::Resume, "Resume"});
            options.push_back({ID::Options, "Options"});
            options.push_back({ID::Exit, "Exit"});

            Rectangle area = {
                64,
                64,
                g::video.getSize().x / 2 - 128,
                g::video.getSize().y - 128
            };

            sf::RectangleShape sh = area;
            sh.setFillColor(sf::Color(32, 32, 32));
            sh.setOutlineThickness(4);
            sh.setOutlineColor(sf::Color::White);
            g::video.draw(sh);


            int res = Menu::List(options, &hover, 0, area);

            if(res == Menu::Accept) {

                if(options[hover].id == ID::Resume) {
                    pause = false;

                }else if(options[hover].id == ID::Options) {
                    Options::run({area.x + 128, area.y, area.w, area.h});

                }else if(options[hover].id == ID::Exit) {
                    return false;
                }

            }else if(res == Menu::Decline) {
                pause = false;
            }
        }
        g::video.display();

        if(game.done())
            return true;
    }
    return false;
}