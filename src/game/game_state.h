#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "core/stage.h"
#include "core/player.h"

struct Game {

	enum Mode {
		Versus,
		Rounds
	};

	enum Flow {
		Done,
		Countdown,
		PlayRound,
		RematchScreen,
		KO,
		DoubleKO,
		TimeUp,
		RoundsScreen
	};

	// Game Mode: Rounds
	struct __Rounds__Choice {
		bool isDefault = false;
		std::string move = "";
		std::string motion = "";
	};

	static const int __Rounds__confSize = Move::Total - Move::Custom00;

	std::vector<__Rounds__Choice> __Rounds__choices;
	void __Rounds__loadChoices();
	void __Rounds__resetPlayerConfigs();
	void __Rounds__fixPlayerConfigs();
	void __Rounds__prepareChoices();
	std::vector<int> __Rounds__getQualifiedChoices(int choice);

	// Game Constants
	int gameMode;
	int playerCount;
	int roundMax;
	int timerMax;
	Stage stage;

	// Player State Control
	Player players[MAX_PLAYERS];

	struct State {

		// General state control
		int flow				= Flow::Done;
		int timer 				= 0;

		// Music Control
		bool songNormal			= false;
		bool songClimax			= false;

		// Match Control
		int slomo				= 0;
		int round				= 0;
		int lWins				= 0;
		int rWins				= 0;
		int winner				= -1;
		int rematch				[MAX_PLAYERS];

		// Mode::Rounds specific state variables
		int roundsConf			[MAX_PLAYERS][__Rounds__confSize];
		int roundsChooser		= -1;
		int roundsChoice		[4];
		int roundsChoicePage	= 0;
	}state;

	struct SaveState {
		Game::State game;
		Player::State players[MAX_PLAYERS];
	};

	void init(int _playerCount, int _roundMax = 3, int _timerMax = 99, int _gameMode = Mode::Versus);
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