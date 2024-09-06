#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "core/player.h"

struct Game {
	int playerCount;
	int roundMax;
	int timerMax;

	Player players[MAX_PLAYERS];

	struct State {
		bool done 		= false;
		bool gameOver 	= false;
		int rematch	[MAX_PLAYERS];
		bool judge		= false;
		int round		= 0;
		int lWins		= 0;
		int rWins		= 0;
		int timer 		= 0;
	}state;

	struct SaveState {
		Game::State game;
		Player::State players[MAX_PLAYERS];
	};

	void init(int _playerCount, int _roundMax = 3, int _timerMax = 99);
	bool done();

	void resetGame();
	void resetRound();
	void nextRound();

	SaveState saveState();
	void loadState(SaveState state);

	void advanceFrame();

	void draw();
};

#endif