#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H

#include <vector>
#include <string>

#include "core/render_instance.h"
#include "core/skeleton.h"
#include "core/clothing.h"
#include "core/animation.h"

using std::vector;
using std::string;

namespace Button {
	enum {
		Up,
		Down,
		Left,
		Right,
		A,
		B,
		C,
		D,
		Taunt,
		Total
	};
};

namespace Move {
	enum {
		// Inherent Stances
		Crouch,

		Stand,
		WalkForwards,
		WalkBackwards,

		Jump,
		JumpForwards,
		JumpBackwards,

		StandBlock,
		CrouchBlock,

		CrouchCombo,
		StandCombo,
		JumpCombo,		

		KnockDown,
		GetUp,

		// Custom Moves
		Custom00,
		Custom01,
		Custom02,
		Custom03,
		Custom04,
		Custom05,
		Custom06,
		Custom07,
		Custom08,
		Custom09,
		Custom10,
		Custom11,
		Custom12,
		Custom13,
		Custom14,
		Custom15,
		Custom16,
		Custom17,
		Custom18,
		Custom19,
		Custom20,
		Custom21,
		Custom22,
		Custom23,
		Custom24,

		Total
	};
};

struct Player {

	Player* opponent;

	struct Config {
		vector<string> clothes;
		string move[Move::Total];
		string motion[Move::Total];
		int button[Button::Total];
	} config;

	struct State {
		int health;
		int stun;
		int hitKeyFrame;
		int side;
		Vector2 position;
		Vector2 velocity;
		int moveIndex;
		int moveFrame;
		bool button[Button::Total];

		Config* config = NULL;

		Vector2 getSOCD();
		string getMotion();
		bool inMove(int move);
		void setMove(int move, bool loop = false);
		bool doneMove();
		int getKeyFrame();
		Frame getFrame();
		Skeleton getSkeleton();
		vector<HitBox> getHitBoxes();
		vector<HurtBox> getHurtBoxes();
	};

	vector<State> history;

	Player();

	void readInput(int gameFrame);
	State& getState(int gameFrame, bool copy = false);	
	State& getNextState(int gameFrame);
	void draw(int gameFrame);

	vector<Clothing*> getClothes();
};

#endif