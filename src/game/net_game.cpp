#include "net_game.h"

#include "core/save.h"
#include "core/render_instance.h"
#include "core/input_interpreter.h"
#include "core/player.h"
#include "core/net_tools.h"

#include <iostream>
#include <cstdio>
#include <ggponet.h>

GGPOSession* 		ggpo;
GGPOPlayerHandle 	handle[2];
Player 				player[2];

struct GameState {
    Player::State p1;
    Player::State p2;
};

bool beginGame(const char *game) {
    return true;
}

bool advanceFrame(int flags) {
    Button::Flag in[2];
    int disconnect_flags;
    GGPOErrorCode result;

    result = ggpo_synchronize_input(ggpo, in, sizeof(Button::Flag) * 2, &disconnect_flags);

    if(result == GGPO_OK) {
        vector<Player> old = {player[0], player[1]};
        player[0].advanceFrame(in[0], old);
        player[1].advanceFrame(in[1], old);
	    ggpo_advance_frame(ggpo);
	}

    return true;
}

bool loadGameState(unsigned char *buffer, int len) {
    GameState* state = (GameState*)buffer;
    player[0].state = state->p1;
    player[1].state = state->p2;
    return true;
}

bool saveGameState(unsigned char **buffer, int *len, int *checksum, int frame) {
    GameState* state = new GameState();
    state->p1 = player[0].state;
    state->p2 = player[1].state;

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

    result = ggpo_start_session(&ggpo, &cb, "CustomFighter", 2, sizeof(Button::Flag), g::save.port);
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
        player[i].id = i;

        // Hint to GGPO what each player is
        GGPOPlayer hint;
        hint.size = sizeof(GGPOPlayer);
        hint.player_num = (i + 1);

        // Determine player type from ip:port
        if(room.remotes[i] == "localhost") {
            hint.type = GGPO_PLAYERTYPE_LOCAL;
            player[i].local_id = 0;

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
        player[0].state.position.x = -75;
        player[0].state.target = 1;
        player[1].state.position.x = 75;    
        player[1].state.target = 0;    
    }
/*
    if(room.remotes.size() == 4) {
        player[0].state.position.x = -75;
        player[1].state.position.x = -75;
        player[2].state.position.x = 75;
        player[3].state.position.x = 75;                        
    }  
*/  
    return true;
}

bool loop() {

    while(g::video.window.isOpen()) {
        g::input.prepEvents();

        sf::Event event;
        while(g::video.window.pollEvent(event)) {
            g::input.processEvent(event);
        }

        if(g::input.windowClose)
            g::video.window.close();

        g::video.window.clear();

        // Camera
        g::video.camera.x = (player[0].state.position.x + player[1].state.position.x) / 2. - g::video.camera.w / 2.;
        g::video.camera.y = g::video.camera.h * 0.75;

        //drawHealthBars(g::video, p[0], p[1]);
        
        // Get buttons for local and remote player
        GGPOErrorCode result;
        Button::Flag in[2];

        if(player[0].local_id != -1) {
            in[0] = player[0].readInput();  
            result = ggpo_add_local_input(ggpo, handle[0], &in[0], sizeof(Button::Flag)); 
        }else {
            in[1] = player[1].readInput();  
            result = ggpo_add_local_input(ggpo, handle[1], &in[1], sizeof(Button::Flag));             
        }

        // Only advance frame when synchronize both player's buttons
        if(result == GGPO_OK) {
            int d;
            result = ggpo_synchronize_input(ggpo, in, sizeof(Button::Flag) * 2, &d);

            if(result == GGPO_OK) {
                vector<Player> old = {player[0], player[1]};
                player[0].advanceFrame(in[0], old);
                player[1].advanceFrame(in[1], old);
                ggpo_advance_frame(ggpo);
            }
        }

        player[0].draw();
        player[1].draw();

        ggpo_idle(ggpo, 16);

        g::video.window.display();
    }
	return true;
}

bool close() {
	ggpo_close_session(ggpo);
	ggpo_deinitialize_winsock();
	return true;
}

bool NetGame::run(Lobby::Room room) {

    if(verify_UDP_hole_punch(g::save.port, room.remotes)) {

        if(start(room)) {
            loop();
            close();
            return true;
        }        
    }
    return false;
}