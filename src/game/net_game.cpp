#include "net_game.h"
#include "game_tools.h"

#include "core/save.h"
#include "core/render_instance.h"
#include "core/input_interpreter.h"
#include "core/player.h"
#include "core/net_tools.h"

#include <iostream>
#include <cstdio>
#include <ggponet.h>

using std::vector, std::string;

GGPOSession* 		ggpo;
int                 currPlayers = 0;
const int           maxPlayers = 4;
GGPOPlayerHandle    handle[maxPlayers];
Player              player[maxPlayers];

struct GameState {
    Player::State p[maxPlayers];
};

bool beginGame(const char *game) {
    return true;
}

bool advanceFrame(int flags) {
    Button::Flag in[maxPlayers];
    int disconnect_flags;
    GGPOErrorCode result;

    result = ggpo_synchronize_input(ggpo, in, sizeof(Button::Flag) * currPlayers, &disconnect_flags);

    if(result == GGPO_OK) {
        vector<Player> others;

        for(int i = 0; i < currPlayers; i ++)
            others.push_back(player[i]);

        for(int i = 0; i < currPlayers; i ++)
            player[i].advanceFrame(in[i], others);

	    ggpo_advance_frame(ggpo);
	}

    return true;
}

bool loadGameState(unsigned char *buffer, int len) {
    GameState* state = (GameState*)buffer;

    for(int i = 0; i < maxPlayers; i ++)
        player[i].state = state->p[i];

    return true;
}

bool saveGameState(unsigned char **buffer, int *len, int *checksum, int frame) {
    GameState* state = new GameState();

    for(int i = 0; i < maxPlayers; i ++)
        state->p[i] = player[i].state;

    *buffer = (unsigned char*)state;
    *len = sizeof(GameState);

    return true;
}

void freeBuffer (void *buffer) {
    GameState* state = (GameState*)buffer;
    delete state;
}

bool onEvent(GGPOEvent *info) {

    switch (info->code) {

    case GGPO_EVENTCODE_CONNECTED_TO_PEER:
        std::cout << "GGPO Connected to Peer\n";
        break;

    case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
        std::cout << "GGPO Synchronizing\n";        
        break;

    case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
        std::cout << "GGPO Synchronized!\n";        
        break;

    case GGPO_EVENTCODE_RUNNING:
        std::cout << "GGPO Running\n";        
        break;

    case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
        std::cout << "GGPO Connection Interupted\n";        
        break;

    case GGPO_EVENTCODE_CONNECTION_RESUMED:
        std::cout << "GGPO Connection Resumed\n";        
        break;

    case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
        std::cout << "GGPO Disconnected from Peer\n";        
        break;

    case GGPO_EVENTCODE_TIMESYNC:
        std::cout << "GGPO Timesync\n";  
        sf::sleep(sf::milliseconds(1000 * info->u.timesync.frames_ahead / 60));      
        break;
    }
   return true;
}

bool start(Lobby::Room room) {

	GGPOErrorCode result;

    // Initiate the GGPO session
	GGPOSessionCallbacks cb;
	cb.begin_game = beginGame;
	cb.advance_frame = advanceFrame;
	cb.load_game_state = loadGameState;
	cb.save_game_state = saveGameState;
	cb.free_buffer = freeBuffer;
	cb.on_event = onEvent;

	// Initialize windows networking
	result = ggpo_initialize_winsock();

    if(result != GGPO_OK) {
    	std::cerr << "GGPO: Failed to initialize windows sockets\n";
    	return false;
    }

    result = ggpo_start_session(&ggpo, &cb, "CustomFighter", 2, sizeof(Button::Flag), g::save.getPort());
	//result = ggpo_start_synctest(&ggpo, &cb, "CustomFighter", 2, sizeof(Button::Flag), 1);

    if(result != GGPO_OK) {
    	std::cerr << "GGPO: Failed to start session\n";
    	return false;
    }

	ggpo_set_disconnect_timeout(ggpo, 3000);
	ggpo_set_disconnect_notify_start(ggpo, 1000);

    // Initialize players
    for(int i = 0; i < room.remotes.size(); i ++) {

        // Reset player states
        player[i] = Player();
        player[i].config = room.configs[i];
        player[i].gameIndex = i;

        // Hint to GGPO what each player is
        GGPOPlayer hint;
        hint.size = sizeof(GGPOPlayer);
        hint.player_num = (i + 1);

        // Determine player type from ip:port
        if(room.remotes[i] == "localhost") {
            hint.type = GGPO_PLAYERTYPE_LOCAL;
            player[i].seatIndex = 0;

        }else {
            hint.type = GGPO_PLAYERTYPE_REMOTE;

            // Get Address components
            int ip[4];
            int port;
            std::sscanf(room.remotes[i].c_str(), "%d.%d.%d.%d:%d", &ip[0], &ip[1], &ip[2], &ip[3], &port);

            // Set the hint components
            std::sprintf(hint.u.remote.ip_address, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
            hint.u.remote.port = port;
        }

        // Try adding player
        result = ggpo_add_player(ggpo, &hint, &handle[i]);

        if(result != GGPO_OK) {
            std::cerr << "GGPO: Failed to add local player\n";
            return false;
        }
    }

    // GGPO done, now set state of players based on room size
    if(room.remotes.size() == 2) {
        currPlayers = 2;

        player[0].state.position.x = -75;
        player[0].state.target = 1;
        player[1].state.position.x = 75;    
        player[1].state.target = 0;    


    }else if(room.remotes.size() == 4) {
        currPlayers = 4;

        player[0].state.position.x = -75;
        player[1].state.position.x = -75;
        player[2].state.position.x = 75;
        player[3].state.position.x = 75;                        
    }  
    return true;
}

bool loop() {

    while(g::video.isOpen()) {
        g::input.pollEvents();
        
        // Get buttons for local and remote player
        GGPOErrorCode result;

        for(int i = 0; i < currPlayers; i ++) {

            if(player[i].seatIndex >= 0) {
                Button::Flag in = player[i].readInput();
                result = ggpo_add_local_input(ggpo, handle[i], &in, sizeof(Button::Flag));
            }
        }

        // Advance frame if local input was OK
        if(result == GGPO_OK) 
            advanceFrame(NULL);

        g::video.clear();

        setCamera(player, currPlayers);

        drawStage(0);
        drawHealthBars(player, currPlayers);

        for(int i = 0; i < currPlayers; i ++) 
            player[i].draw();

        g::video.display();

        ggpo_idle(ggpo, 16);
    }
	return true;
}

bool close() {
	ggpo_close_session(ggpo);
	ggpo_deinitialize_winsock();
	return true;
}

bool NetGame::run(Lobby::Room room) {

    if(verify_UDP_hole_punch(g::save.getPort(), room.remotes)) {

        if(start(room)) {
            loop();
            close();
            return true;
        }
    }
    return false;
}