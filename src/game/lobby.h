#ifndef NET_LOBBY_H
#define NET_LOBBY_H

#include "core/player.h"

namespace Lobby {

	struct Room {
		int code = -1;
		int refresh = 0;
		vector<Player::Config> 	configs;
		vector<string>			remotes;
	};

	bool run(Room& room);
};

#endif