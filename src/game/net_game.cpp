#include "net_game.h"
#include "game_state.h"

#include "core/save.h"
#include "core/render_instance.h"
#include "core/input_interpreter.h"
#include "core/audio.h"
#include "core/player.h"
#include "core/net_tools.h"

#include <iostream>
#include <cstdio>
#include <ggponet.h>

using std::vector, std::string;

GGPOSession* 		ggpo;
GGPOPlayerHandle    handle[MAX_PLAYERS];
Game                game;

bool beginGame(const char *game) {
    return true;
}

bool advanceFrame(int flags) {
    Button::Flag in[MAX_PLAYERS];
    int disconnect_flags;
    GGPOErrorCode result;

    result = ggpo_synchronize_input(ggpo, in, sizeof(Button::Flag) * MAX_PLAYERS, &disconnect_flags);

    if(result == GGPO_OK) {

        // Mute sounds while in rollback to prevent repeats
        g::audio.disable();

        for(int i = 0; i < game.playerCount; i ++) 
            game.players[i].in = in[i];

        game.advanceFrame();

        // Restore default volume
        g::audio.enable();

	    ggpo_advance_frame(ggpo);
	}

    return true;
}

bool loadGameState(unsigned char *buffer, int len) {
    Game::SaveState* state = (Game::SaveState*)buffer;

    game.loadState(*state);

    return true;
}

bool saveGameState(unsigned char **buffer, int *len, int *checksum, int frame) {
    Game::SaveState* state = new Game::SaveState();

    *state = game.saveState();

    *buffer = (unsigned char*)state;
    *len = sizeof(Game::SaveState);
    return true;
}

void freeBuffer (void *buffer) {
    Game::SaveState* state = (Game::SaveState*)buffer;
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

    result = ggpo_start_session(&ggpo, &cb, "CustomFighter", room.player_count, sizeof(Button::Flag), g::save.getPort());
	//result = ggpo_start_synctest(&ggpo, &cb, "CustomFighter", 2, sizeof(Button::Flag), 1);

    if(result != GGPO_OK) {
    	std::cerr << "GGPO: Failed to start session\n";
    	return false;
    }

	ggpo_set_disconnect_timeout(ggpo, 3000);
	ggpo_set_disconnect_notify_start(ggpo, 1000);

    // Configure players
    for(int i = 0; i < room.player_count; i ++) {

        // Reset player states
        game.players[i].config = room.player_data[i].config;

        // Hint to GGPO what each player is
        GGPOPlayer hint;
        hint.size = sizeof(GGPOPlayer);
        hint.player_num = (i + 1);

        // Determine player type from ip:port
        if(room.player_data[i].remote == "localhost") {
            game.players[i].seatIndex = 0;            
            hint.type = GGPO_PLAYERTYPE_LOCAL;

        }else {
            game.players[i].seatIndex = -1;
            hint.type = GGPO_PLAYERTYPE_REMOTE;

            // Get Address components
            int ip[4];
            int port;
            std::sscanf(room.player_data[i].remote.c_str(), "%d.%d.%d.%d:%d", &ip[0], &ip[1], &ip[2], &ip[3], &port);

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

    // GGPO done, initialize the game
    game.init(room.player_count);

    return true;
}

bool loop() {

    while(g::video.isOpen()) {
        g::input.pollEvents();
        
        // Get button flags for local players
        GGPOErrorCode result;

        for(int i = 0; i < game.playerCount; i ++) {

            if(game.players[i].seatIndex >= 0) {
                Button::Flag in = game.players[i].readInput();
                result = ggpo_add_local_input(ggpo, handle[i], &in, sizeof(Button::Flag));
            }
        }

        // Advance frame if local input was OK
        if(result == GGPO_OK) 
            advanceFrame(NULL);

        g::video.clear();
        game.draw();
        g::video.display();

        if(game.done())
            return true;

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

    // Collect remotes to test UDP
    vector<string> remotes;

    for(int i = 0; i < room.player_count; i ++) {

        if(room.player_data[i].remote != "")
            remotes.push_back(room.player_data[i].remote);
    }

    if(!verify_UDP_hole_punch(g::save.getPort(), remotes))
        return false;

    if(!start(room)) {
        close();
        return false;
    }

    loop();
    close();
    return true;
}