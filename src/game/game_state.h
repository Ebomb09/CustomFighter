#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "core/stage.h"
#include "core/player.h"

const std::vector<std::string> PredefinedMotions {
	"",
	"6",
	"4",
	"2",
	"3",
	"1",
	"66",
	"44",
	"236",
	"623",
	"214",
	"421",
	"236236",
	"214214"
	"41236",
	"63214",
	"2141236",
	"2363214",
	"632146",
	"1632143"
};

const std::vector<std::string> PredefinedButtons {
	"A",
	"B",
	"A+B"
};

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
		int flow			= Flow::Done;
		int timer 			= 0;

		// Music Control
		bool songNormal		= false;
		bool songClimax		= false;

		// Match Control
		int slomo			= 0;
		int round			= 0;
		int lWins			= 0;
		int rWins			= 0;
		int rematch			[MAX_PLAYERS];

		// Mode::Rounds specific state variables
		int confMove		[MAX_PLAYERS][Move::Total];
		int confMotion		[MAX_PLAYERS][Move::Total];
		int confButton		[MAX_PLAYERS][Move::Total];
		int roundsChoice	[4][3];
		int roundsChooser	= 0;
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

	void __Rounds__resetPlayerConfigs();
	void __Rounds__fixPlayerConfigs();
	void __Rounds__prepareChoices();
	std::vector<std::string> __Rounds__getAnimations();
	std::vector<int> __Rounds__getQualifiedAnimations(int choice);
	std::vector<int> __Rounds__getQualifiedMotions(int choice);
	std::vector<int> __Rounds__getQualifiedButtons(int choice);

	SaveState saveState();
	void loadState(SaveState state);

	void readInput();
	void advanceFrame();

	void draw();
};

#endif