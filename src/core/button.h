#ifndef GAME_PLAYER_BUTTON_LAYOUT_H
#define GAME_PLAYER_BUTTON_LAYOUT_H

#include <string>
#include <filesystem>

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

	struct Config {
		int index;

		union {
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

		Config();

		bool loadFromFile(std::filesystem::path path);
		void saveToFile(std::filesystem::path path);		
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