#include "lobby.h"
#include "local_game.h"
#include "net_game.h"
#include "character_select.h"
#include "options.h"

#include "core/menu.h"
#include "core/input_interpreter.h"
#include "core/video.h"
#include "core/save.h"

#ifdef __WIN32__
    #include <windows.h>
#endif

using std::vector, std::string;

enum {
    VersusFight2P,
    VersusFight4P,
    RoundsFight2P,
    RoundsFight4P,
    NetPlay,
    OptionMenu,
    Exit
};

vector<Menu::Option> menuOptions = {
    {NetPlay, "NetPlay"},    
    {VersusFight2P, "Versus Mode: 2P Fight"},
    {VersusFight4P, "Versus Mode: 4P Fight"},
    {RoundsFight2P, "Rounds Mode: 2P Fight"},
    {RoundsFight4P, "Rounds Mode: 4P Fight"},
    {OptionMenu, "Options"},
    {Exit, "Exit"}
};
int hover = 0;

int main(int argc, char* argv[]) {

    #ifdef __WIN32__
        ShowWindow(GetConsoleWindow(), SW_HIDE);   
    #endif

    g::video.setTitle("Custom Fighter");
    g::video.setSize(g::save.resolution);
    g::video.setDisplayMode(g::save.displayMode);
    g::video.setVSync(g::save.vsync);
    g::video.reload();

    while (g::video.isOpen()) {
        g::input.pollEvents();

        g::video.clear();

        int res = Menu::List(menuOptions, &hover, 0, {0, 0, (float)g::video.getSize().x, (float)g::video.getSize().y});

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
                
            }else if(menuOptions[hover].id == NetPlay) {
                vector<Player::Config> configs = CharacterSelect::run(1);

                if(configs.size() == 1) {
                    Lobby::Room room = Lobby::run(configs[0]);

                    if(room.good()) {
                        NetGame::run(room);
                    }
                }

            }else if(menuOptions[hover].id == OptionMenu) {
                Options::run();
            
            }else if(menuOptions[hover].id == Exit) {
                g::video.close();
            }
        }

        g::video.display();
    }

	return 0;
}