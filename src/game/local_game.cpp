#include "local_game.h"
#include "game_tools.h"

#include "core/input_interpreter.h"
#include "core/render_instance.h"

bool LocalGame::run(vector<Player::Config> configs) {
	vector<Player> players;

    for(int i = 0; i < configs.size(); i ++) {
        Player add;
        add.gameIndex = i;
        add.seatIndex = i;
        add.config = configs[i];
        players.push_back(add);
    }

    // 2 Player Game
    if(players.size() == 2) {
        players[0].state.position = -75;
        players[0].state.target = 1;
        players[1].state.position = 75; 
        players[1].state.target = 0;

    // 4 Player Tag Game
    } else if(players.size() == 4) {
        players[0].state.position = -75;
        players[1].state.position = -75;        
        players[2].state.position = 75; 
        players[3].state.position = 75; 
    }

    while(g::video.isOpen()) {
        g::input.pollEvents();

        g::video.clear();

        setCamera(players);

        drawStage(0);
        drawHealthBars(players);

        // Update players
        vector<Player> old = players;

        for(Player& ply : players) {
            ply.advanceFrame(ply.readInput(), old);
            ply.draw();
        }

        g::video.display();
    }
    return true;
}