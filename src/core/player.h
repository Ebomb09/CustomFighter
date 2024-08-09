#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H

#include <vector>
#include <string>

#include "skeleton.h"
#include "clothing.h"
#include "animation.h"

#include "button.h"

using std::vector;
using std::string;

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

	int id = 0;
	int local_id = -1;

	struct Config {
		vector<string>	clothes;
		string 			moves			[Move::Total];
		string 			motions			[Move::Total];

		void loadFromText(string str);
		string saveToText();
		void loadFromFile(std::string fileName);
		void saveToFile(std::string fileName);
	}config;

	struct State {
		int 			target			= -1;
		int				health			= 100;
		int				accDamage		= 0;
		int				stun			= 0;
		int				hitStop			= 0;
		int				hitKeyFrame		= -1;
		int				side			= 1;
		Vector2			position		= {0, 0};
		Vector2			velocity		= {0, 0};
		int				moveIndex		= Move::Stand;
		int				moveFrame		= 0;
		float 			look			= 0;
		Button::Flag	button			[Button::History];

	}state;

	Button::Flag readInput();
	void advanceFrame(Button::Flag in, vector<Player> others);
	void draw();

	void dealDamage(int dmg);

	Vector2 getSOCD(int index = 0);
	string getMotion(int index = 0);

	bool inMove(int move);
	void setMove(int move, bool loop = false);
	bool doneMove();

	int getKeyFrame();
	Frame getFrame();
	
	Skeleton getSkeleton();
	vector<HitBox> getHitBoxes();
	vector<HurtBox> getHurtBoxes();	

	vector<Clothing*> getClothes();	
};

#endif