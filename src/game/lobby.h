#ifndef NET_LOBBY_H
#define NET_LOBBY_H

#include "game_state.h"
#include "core/player.h"

namespace Lobby {

	struct PlayerData {
		std::string remote;
		Player::Config config;
	};

	struct Room {
		int code = -1;
		std::string password = "";
		int player_max = 2;
		int player_count = 0;
		PlayerData player_data[MAX_PLAYERS];

		bool good();
	};

	Room run(Player::Config config);
};

#endif