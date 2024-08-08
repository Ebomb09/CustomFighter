#ifndef GAME_PLAYER_BUTTON_LAYOUT_H
#define GAME_PLAYER_BUTTON_LAYOUT_H

struct Button {
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
		Total,
		History = 30
	};

	union Config {
		struct {
			int Up;
			int Down;
			int Left;
			int Right;
			int A;
			int B;
			int C;
			int D;
			int Taunt;	
		};
		int button[Button::Total];
	};	

	struct Flag {
		unsigned int Up 	: 1;
		unsigned int Down 	: 1;
		unsigned int Left 	: 1;
		unsigned int Right 	: 1;
		unsigned int A 		: 1;
		unsigned int B 		: 1;
		unsigned int C 		: 1;
		unsigned int D 		: 1;
		unsigned int Taunt 	: 1;
	};	
};

#endif