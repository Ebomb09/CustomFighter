#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H

#include <vector>
#include <string>

#include "core/skeleton.h"
#include "core/clothing.h"
#include "core/animation.h"

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

struct Button {
	int 			Up				= 0;
	int 			Down			= 0;
	int 			Left			= 0;
	int 			Right			= 0;
	int 			A				= 0;
	int 			B				= 0;
	int 			C				= 0;
	int 			D				= 0;
	int 			Taunt			= 0;

	static const int History = 30; 
};

struct Player {

	struct Config {
		vector<string> 	clothes;
		string 			move			[Move::Total];
		string 			motion			[Move::Total];
		Button 			button;
		Player*			opponent		= NULL;

	}config;

	struct State {
		int 			health			= 100;
		int				accDamage		= 0;
		int 			stun			= 0;
		int 			hitStop			= 0;
		int 			hitKeyFrame		= -1;
		int 			side			= 1;
		Vector2 		position		= {0, 0};
		Vector2 		velocity		= {0, 0};
		int 			moveIndex		= Move::Stand;
		int 			moveFrame		= 0;
		Button		 	button			[30];

	}state;

	Button readInput();
	void advanceFrame(Button in);
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