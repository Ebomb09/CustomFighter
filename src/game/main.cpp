#include "lobby.h"
#include "local_game.h"
#include "net_game.h"

#include "core/save.h"
#include "core/render_instance.h"

int main(int argc, char* argv[]) {

    g::video.init(1024, 768, "Fighting Room Test");
    g::video.camera.w /= 4;
    g::video.camera.h /= 4;

    Lobby::Room room;

    //LocalGame::run(g::save.getPlayerConfig(0), g::save.getPlayerConfig(0));

    if(Lobby::run(room)) {
        NetGame::run(room);
    }

	return 0;
}