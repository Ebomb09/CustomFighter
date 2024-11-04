#include "lobby.h"
#include "local_game.h"
#include "net_game.h"
#include "practise_game.h"
#include "character_select.h"
#include "options.h"

#include "core/menu.h"
#include "core/input_interpreter.h"
#include "core/video.h"
#include "core/save.h"

#include <iostream>

using std::vector, std::string;

enum {
    VersusFight2P,
    VersusFight4P,
    RoundsFight2P,
    RoundsFight4P,
    NetPlay,
    Practise,
    OptionMenu,
    Exit
};

Vector2 pointsOfInterests[] {
    {275.f, -307.f},
    {817.f, -314.f},
    {817.f, -314.f},
    {817.f, -314.f},
    {817.f, -314.f},
    {588.f, -355.f},
    {1050.f, -615.f},
    {588.f, -355.f}
};

vector<Menu::Option> menuOptions = {
    {NetPlay, "NetPlay"},    
    {VersusFight2P, "Versus Mode: 2P Fight"},
    {VersusFight4P, "Versus Mode: 4P Fight"},
    {RoundsFight2P, "Rounds Mode: 2P Fight"},
    {RoundsFight4P, "Rounds Mode: 4P Fight"},
    {Practise, "Practise Mode"},
    {OptionMenu, "Options"},
    {Exit, "Exit"}
};
int hover = 0;

int main(int argc, char* argv[]) {
    g::video.setTitle("Custom Fighter");
    g::video.setSize(g::save.resolution);
    g::video.setDisplayMode(g::save.displayMode);
    g::video.setVSync(g::save.vsync);
    g::video.reload();

    // Title screen
    sf::Texture* mainBG = g::save.getTexture("data/hud/main.png");
    int waitForInterest = -1;

    while (g::video.isOpen()) {
        g::input.pollEvents();
        g::video.clear();

        // Draw the main bg
        if(mainBG) {

            Vector2 maxSize;

            if(g::video.getSize().x / g::video.getSize().y < (float)mainBG->getSize().x / (float)mainBG->getSize().y) {
                maxSize = {
                    (float)mainBG->getSize().y * g::video.getSize().x / g::video.getSize().y,
                    (float)mainBG->getSize().y
                };

            }else {
                maxSize = {
                    (float)mainBG->getSize().x,                    
                    (float)mainBG->getSize().x * g::video.getSize().y / g::video.getSize().x,
                };
            }

            if(waitForInterest != hover) 
                waitForInterest = -1;

            Vector2 desiredSize;
            Vector2 desiredPoint; 
            
            if(waitForInterest == -1) {
                desiredSize = {maxSize.x, maxSize.y};
                desiredPoint = {mainBG->getSize().x / 2.f, mainBG->getSize().y / -2.f};

            }else {
                desiredSize = {maxSize.x / 2.f, maxSize.y / 2.f};
                desiredPoint = pointsOfInterests[hover];
            }

            // Can pick a new interest
            if(waitForInterest == -1 && g::video.camera.w / desiredSize.x > 0.75f) 
                waitForInterest = hover;

            // Get center of camera
            Vector2 center = {g::video.camera.x + g::video.camera.w / 2.f, g::video.camera.y - g::video.camera.h / 2.f};

            // Adjust camera to the desired
            center.x += (desiredPoint.x - center.x) * 0.05f;
            center.y += (desiredPoint.y - center.y) * 0.05f;
            g::video.camera.w += (desiredSize.x - g::video.camera.w) * 0.05f;
            g::video.camera.h += (desiredSize.y - g::video.camera.h) * 0.05f;

            // Readjust camera to center
            g::video.camera.x = center.x - g::video.camera.w / 2.f;
            g::video.camera.y = center.y + g::video.camera.h / 2.f;

            g::video.camera.x = std::clamp(g::video.camera.x, 0.f, mainBG->getSize().x - g::video.camera.w);
            g::video.camera.y = std::clamp(g::video.camera.y, mainBG->getSize().y * -1.0f + g::video.camera.h, 0.f);
            g::video.camera.w = std::clamp(g::video.camera.w, 0.f, maxSize.x);
            g::video.camera.h = std::clamp(g::video.camera.h, 0.f, maxSize.y);

            sf::RectangleShape rect = g::video.toScreen({0.f, 0.f, (float)mainBG->getSize().x, (float)mainBG->getSize().y});
            rect.setTexture(mainBG);
            g::video.draw(rect);
        }

        Rectangle area = {
            64,
            64,
            g::video.getSize().x / 2 - 128,
            g::video.getSize().y - 128
        };

        int res = Menu::List(menuOptions, &hover, 0, area);

        if(res == Menu::Accept) {

            if(menuOptions[hover].id == VersusFight2P) {
                vector<Player::Config> configs = CharacterSelect::run(2);

                if(configs.size() == 2) {
                    LocalGame::run(configs);
                }
                
            }else if(menuOptions[hover].id == VersusFight4P) {
                vector<Player::Config> configs = CharacterSelect::run(4);

                if(configs.size() == 4) {
                    LocalGame::run(configs);
                }
                
            }else if(menuOptions[hover].id == RoundsFight2P) {
                vector<Player::Config> configs = CharacterSelect::run(2);

                if(configs.size() == 2) {
                    LocalGame::run(configs, GameMode::Rounds);
                }
                
            }else if(menuOptions[hover].id == RoundsFight4P) {
                vector<Player::Config> configs = CharacterSelect::run(4);

                if(configs.size() == 4) {
                    LocalGame::run(configs, GameMode::Rounds);
                }
                
            }else if(menuOptions[hover].id == Practise) {
                vector<Player::Config> configs = CharacterSelect::run(2);

                if(configs.size() == 2) {
                    PractiseGame::run(configs);
                }
                
            }else if(menuOptions[hover].id == NetPlay) {
                vector<Player::Config> configs = CharacterSelect::run(1);

                if(configs.size() == 1) {
                    Lobby::Room room = Lobby::run(configs[0]);

                    if(room.good()) {
                        NetGame::run(room);
                    }
                }

            }else if(menuOptions[hover].id == OptionMenu) {
                Options::run({area.x + 128, area.y, area.w, area.h});
            
            }else if(menuOptions[hover].id == Exit) {
                g::video.close();
            }
        }

        g::video.display();
    }

	return 0;
}