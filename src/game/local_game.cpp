#include "local_game.h"
#include "game_state.h"

#include "core/input_interpreter.h"
#include "core/video.h"

using std::vector;

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

    while(g::video.isOpen()) {
        g::input.pollEvents();

        game.readInput();
        game.advanceFrame();

        g::video.clear();
        game.draw();
        g::video.display();

        if(game.done())
            return true;
    }
    return false;
}