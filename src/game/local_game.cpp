#include "local_game.h"
#include "game_state.h"

#include "core/input_interpreter.h"
#include "core/render_instance.h"

using std::vector;

bool LocalGame::run(vector<Player::Config> configs) {
	Game game;
    game.init(configs.size());

    // Configure players
    for(int i = 0; i < configs.size(); i ++) {
        game.players[i].seatIndex = i;
        game.players[i].config = configs[i];
    }

    while(g::video.isOpen()) {
        g::input.pollEvents();

        for(int i = 0; i < game.playerCount; i ++)
            game.players[i].in = game.players[i].readInput();

        game.advanceFrame();

        g::video.clear();
        game.draw();
        g::video.display();

        if(game.done())
            return true;
    }
    return false;
}