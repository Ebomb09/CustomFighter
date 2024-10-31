#include "practise_game.h"
#include "game_state.h"
#include "character_select.h"
#include "options.h"

#include "core/input_interpreter.h"
#include "core/video.h"
#include "core/menu.h"
#include "core/audio.h"

#include <vector>

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

    if(configs.size() != 2)
        return false;

	Game game;

    // Configure players
    for(int i = 0; i < configs.size(); i ++) {
        game.players[i].seatIndex = i;
        game.players[i].config = configs[i];
    }

    game.init(configs.size(), 3, 100);

    bool pause = false;
    int hover = 0;

    vector<int> combo;

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
            vector<Player> others;
            for(int i = 0; i < game.playerCount; i ++)
                others.push_back(game.players[i]);

            game.readInput();
            game.advanceFrame();

            // Ensure the players states
            for(int i = 0; i < game.playerCount; i ++) {
                Player& ply = game.players[i];

                // Reset the player health and combos
                if(
                    ply.state.position.y <= 0 && 
                    ply.state.stun == 0 &&
                    ply.state.hitStop == 0 &&
                    ply.state.grabIndex < 0 &&
                    ply.state.moveIndex < Move::Custom00
                ) {
                    ply.state.health = 100;
                }
            }

            if(game.state.flow == Game::Flow::PlayRound)
                game.state.timer = (game.timerMax - 1) * 60;

            // Add the move to the combo list
            if(game.players[1].state.health < others[1].state.health) {

                if(others[1].state.health == 100)
                    combo.clear();

                combo.push_back(game.players[0].state.moveIndex);
            }
        }

        game.draw();

        // Draw the combo list
        vector<Menu::Option> options;

        for(int i = 0; i < combo.size(); i ++) {
            options.push_back({0, game.players[0].config.motions[combo[i]], "fight", -1});
            options.push_back({0, game.players[0].config.moves[combo[i]], "Anton-Regular", -1});
        }

        Rectangle comboArea = {32, 128, g::video.getSize().x / 4 - 32, g::video.getSize().y - 128};
        Menu::Table(options, 2, false, NULL, 0, comboArea);

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
            sh.setFillColor(sf::Color(32, 32, 32));
            sh.setOutlineThickness(4);
            sh.setOutlineColor(sf::Color::White);
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