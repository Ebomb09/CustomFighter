#ifndef NET_LOBBY_H
#define NET_LOBBY_H

#include "core/player.h"

namespace Lobby {

	struct Room {
		bool good = false;
		int code = -1;
		int refresh = 0;
		vector<Player::Config> 	configs;
		vector<string>			remotes;
	};

	Room run(Player::Config config);
};

#endif