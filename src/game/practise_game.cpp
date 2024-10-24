#include "practise_game.h"
#include "game_state.h"
#include "character_select.h"
#include "options.h"

#include "core/input_interpreter.h"
#include "core/video.h"
#include "core/menu.h"

#include <iostream>

using std::vector;

namespace ID {
    enum {
        Resume,
        QuickSelect,
        Options,
        Exit
    };
};


bool PractiseGame::run(vector<Player::Config> configs) {
	Game game;

    // Configure players
    for(int i = 0; i < configs.size(); i ++) {
        game.players[i].seatIndex = i;
        game.players[i].config = configs[i];
    }

    game.init(configs.size(), 3, 100);

    bool pause = false;
    int hover = 0;

    while(g::video.isOpen()) {
        g::input.pollEvents();
        g::video.clear();

        if(g::input.pressed(KEYBOARD_INDEX, sf::Keyboard::Escape))
            pause = !pause;

        if(!pause) {
            game.readInput();
            game.advanceFrame();

            // Ensure the players states
            for(int i = 0; i < game.playerCount; i ++) {

                if(game.players[i].state.position.y <= 0 && 
                    game.players[i].state.stun == 0 && 
                    game.players[i].state.hitStop == 0 &&
                    game.players[i].state.grabIndex < 0 &&
                    game.players[i].inMove(Move::Stand)) {

                    game.players[i].state.health = 100;
                }
            }

            if(game.state.flow == Game::Flow::PlayRound)
                game.state.timer = (game.timerMax - 1) * 60;
        }
        game.draw();

        if(pause) {
            vector<Menu::Option> options;
            options.push_back({ID::Resume, "Resume"});
            options.push_back({ID::QuickSelect, "Quick Select"});
            options.push_back({ID::Options, "Options"});
            options.push_back({ID::Exit, "Exit"});

            Rectangle area = {
                64,
                64,
                g::video.getSize().x / 2 - 128,
                g::video.getSize().y - 128
            };

            sf::RectangleShape sh = area;
            sh.setFillColor(sf::Color::Black);
            g::video.draw(sh);

            int res = Menu::List(options, &hover, 0, area);

            if(res == Menu::Accept) {

                if(options[hover].id == ID::Resume) {
                    pause = false;

                }else if(options[hover].id == ID::QuickSelect) {
                    vector<Player::Config> edit = CharacterSelect::run(configs.size());

                    if(edit.size() == configs.size()) {

                        // Reset player config and cache
                        for(int i = 0; i < game.playerCount; i ++) {
                            game.players[i].config = edit[i];
                            game.players[i].setState(game.players[i].state);
                        }
                    }

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