#include "lobby.h"
#include "local_game.h"
#include "net_game.h"
#include "character_select.h"
#include "options.h"

#include "core/menu.h"
#include "core/input_interpreter.h"
#include "core/render_instance.h"

using std::vector, std::string;

enum {
    LocalFight2P,
    LocalFight4P,
    NetPlay,
    OptionMenu,
    Exit
};

vector<Menu::Option> menuOptions = {
    {"NetPlay", NetPlay},    
    {"Local 2P Fight", LocalFight2P},
    {"Local 4P Fight", LocalFight4P},
    {"Options", OptionMenu},
    {"Exit", Exit}
};
int menuHover = 0;

int main(int argc, char* argv[]) {

    g::video.init("Custom Fighter");

    while (g::video.isOpen()) {
        g::input.pollEvents();

        g::video.clear();

        int res = Menu::List(menuOptions, &menuHover, 0, {0, 0, (float)g::video.getSize().x, (float)g::video.getSize().y});

        if(res == Menu::Accept) {

            if(menuOptions[menuHover].id == LocalFight2P) {
                vector<Player::Config> configs = CharacterSelect::run(2);

                if(configs.size() == 2) {
                    LocalGame::run(configs);
                }
                
            }else if(menuOptions[menuHover].id == LocalFight4P) {
                vector<Player::Config> configs = CharacterSelect::run(4);

                if(configs.size() == 4) {
                    LocalGame::run(configs);
                }
                
            }else if(menuOptions[menuHover].id == NetPlay) {
                vector<Player::Config> configs = CharacterSelect::run(1);

                if(configs.size() == 1) {
                    Lobby::Room room = Lobby::run(configs[0]);

                    if(room.good()) {
                        NetGame::run(room);
                    }
                }

            }else if(menuOptions[menuHover].id == OptionMenu) {
                Options::run();
            
            }else if(menuOptions[menuHover].id == Exit) {
                g::video.close();
            }
        }

        g::video.display();
    }

	return 0;
}