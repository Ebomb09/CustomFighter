#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "core/player.h"

struct Game {
	Player players[4];

	struct State {
		Player::State states[4];
	};
};

#endif