#ifndef GAME_PLAYER_BUTTON_LAYOUT_H
#define GAME_PLAYER_BUTTON_LAYOUT_H

#include <string>

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

	const std::string String[] = {
		"Up", 
		"Down", 
		"Left", 
		"Right",
		"A", 	
		"B", 	
		"C", 	
		"D", 	
		"Taunt",
		"Total"
	};

	const int History = 30;

	union Config {
		struct {
			int Up		= 0;
			int Down	= 0;
			int Left	= 0;
			int Right	= 0;
			int A		= 0;
			int B		= 0;
			int C		= 0;
			int D		= 0;
			int Taunt	= 0;	
		};
		int button[Button::Total];
	};	

	struct Flag {
		unsigned int Up 	: 1 = 0;
		unsigned int Down 	: 1 = 0;
		unsigned int Left 	: 1 = 0;
		unsigned int Right 	: 1 = 0;
		unsigned int A 		: 1 = 0;
		unsigned int B 		: 1 = 0;
		unsigned int C 		: 1 = 0;
		unsigned int D 		: 1 = 0;
		unsigned int Taunt 	: 1 = 0;
	};	
};

#endif