#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "core/stage.h"
#include "core/player.h"

struct Game {

	enum Judgement {
		None,
		KO,
		DoubleKO,
		TimeUp
	};

	// Game Constants
	int playerCount;
	int roundMax;
	int timerMax;
	Stage stage;

	// State Control
	Player players[MAX_PLAYERS];

	struct State {
		bool done 			= false;
		bool gameOver 		= false;
		bool playJudgement 	= false;
		bool playFight 		= false;
		int rematch	[MAX_PLAYERS];
		int judge			= None;
		int slomo 			= 0;
		int round			= 0;
		int lWins			= 0;
		int rWins			= 0;
		int timer 			= 0;
		bool songNormal		= false;
		bool songClimax		= false;
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

	void readInput();
	void advanceFrame();

	void draw();
};

#endif