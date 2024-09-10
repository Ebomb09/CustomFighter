#ifndef GAME_CHARACTER_SELECT_H
#define GAME_CHARACTER_SELECT_H

#include "core/player.h"

#include <vector>

#define MAX_POINTS 30

namespace CharacterSelect {
	std::vector<Player::Config> run(int count);
};

#endif