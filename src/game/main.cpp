#include "lobby.h"
#include "local_game.h"
#include "net_game.h"
#include "character_select.h"

#include "core/save.h"
#include "core/render_instance.h"

int main(int argc, char* argv[]) {

    g::video.init(1024, 768, "Fighting Room Test");
    g::video.camera.w /= 4;
    g::video.camera.h /= 4;

    Lobby::Room room;

    vector<Player::Config> confs = CharacterSelect::run(2);

    if(confs.size() == 2) 
        LocalGame::run(confs);

    /*if(Lobby::run(room)) {
        NetGame::run(room);
    }*/

	return 0;
}