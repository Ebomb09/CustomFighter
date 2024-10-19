#ifndef LOCAL_GAME_H
#define LOCAL_GAME_H

#include "game_state.h"
#include "core/player.h"

namespace LocalGame {
	bool run(std::vector<Player::Config> configs, int gameMode = Game::Mode::Versus);
};

#endif