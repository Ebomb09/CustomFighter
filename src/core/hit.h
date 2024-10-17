#ifndef GAME_HITS_H
#define GAME_HITS_H

#include "math.h"

namespace GrabBreak {
	
	enum {
		Unbreakable,
		A,
		C,
		AC,
		Total
	};

	const std::string String[] {
		"Unbreakable",
		"A",
		"C",
		"A+C",
		"Total"
	};
};

namespace HitType {

	enum {
		High,
		Mid,
		Low,
		Unblockable,
		Grab,
		Total
	};

	const std::string String[] {
		"High",
		"Mid",
		"Low",
		"Unblockable",
		"Grab",
		"Total"
	};
};

struct HitBox : Rectangle {
	Vector2 force;
	int type = HitType::Mid;
	int damage = 0;
	int hitStun = 0;
	int blockStun = 0;
	bool knockdown = false;
};

struct HurtBox : Rectangle {
	int armour = 0;
};

#endif