#ifndef GAME_PLAYER_MOVE_H
#define GAME_PLAYER_MOVE_H

#include <string>
#include <vector>

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

	const std::string String[] = {
		"Crouch",

		"Stand",
		"WalkForwards",
		"WalkBackwards",

		"Jump",
		"JumpForwards",
		"JumpBackwards",

		"StandBlock",
		"CrouchBlock",

		"CrouchCombo",
		"StandCombo",
		"JumpCombo",

		"KnockDown",
		"GetUp",

		"Custom00",
		"Custom01",
		"Custom02",
		"Custom03",
		"Custom04",
		"Custom05",
		"Custom06",
		"Custom07",
		"Custom08",
		"Custom09",
		"Custom10",
		"Custom11",
		"Custom12",
		"Custom13",
		"Custom14",
		"Custom15",
		"Custom16",
		"Custom17",
		"Custom18",
		"Custom19",
		"Custom20",
		"Custom21",
		"Custom22",
		"Custom23",
		"Custom24",

		"Total"
	};

	std::vector<int> getValidCategories(int move);
};

namespace MoveCategory {

	enum {
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

		Normal,
		CommandNormal,
		Special,
		Super,
		Grab,

		AirNormal,
		AirCommandNormal,
		AirSpecial,
		AirSuper,
		AirGrab,
		
		Total
	};

	const std::string String[] = {
		"Crouch",
		"Stand",
		"WalkForwards",
		"WalkBackwards",
		"Jump",
		"JumpForwards",
		"JumpBackwards",
		"StandBlock",
		"CrouchBlock",
		"CrouchCombo",
		"StandCombo",
		"JumpCombo",		
		"KnockDown",
		"GetUp",

		"Normal",
		"CommandNormal",
		"Special",
		"Super",
		"Grab",

		"AirNormal",
		"AirCommandNormal",
		"AirSpecial",
		"AirSuper",
		"AirGrab",

		"Total"	
	};
}

#endif