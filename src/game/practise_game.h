#ifndef PRACTISE_GAME_H
#define PRACTISE_GAME_H

#include "game_state.h"
#include "core/player.h"

namespace PractiseGame {
	bool run(std::vector<Player::Config> configs);
};

#endif