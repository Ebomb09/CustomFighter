#ifndef GAME_MENU_H
#define GAME_MENU_H

#include "math.h"

#include <vector>
#include <string>

namespace Menu {

	const float fontHeight = 30;

	struct Option {
		std::string name	= "";
		int id				= -1;
		std::string font 	= "Anton-Regular";
	};

	enum {
		Accept,
		Decline,
		Wait
	};

	int Table(std::vector<Option> options, int columns, bool selectByRow, int* hover, int user, Rectangle area);
	int List(std::vector<Option> options, int* hover, int user, Rectangle area);
	int Text(std::string* str, int user, Rectangle area);
	int Motion(std::string* str, int user, Rectangle area);	
};

#endif