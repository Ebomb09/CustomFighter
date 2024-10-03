#ifndef GAME_MENU_H
#define GAME_MENU_H

#include "math.h"
#include "player.h"

#include <vector>
#include <string>

namespace Menu {

	const float fontHeight = 30;

	struct Option {
		int id;
		int data;

		enum class Type {
			Empty,
			Text,
			Player
		} type;

		union {
			struct {
				std::string* text;
				std::string* font;
				int align;
			};

			struct {
				Player* player;
				Rectangle* capture;
			};
		};

		Option();
		Option(int _id, std::string _text, std::string _font = "Anton-Regular", int _align = 0);
		Option(int _id, Player _player, Rectangle _capture = {0,0,0,0});
		Option(int _id, int _data, std::string _text, std::string _font = "Anton-Regular", int _align = 0);
		Option(int _id, int _data, Player _player, Rectangle _capture = {0,0,0,0});		
		~Option();

		Option(Option&& move);
		Option(const Option& copy);
	};

	enum {
		Accept,
		Decline,
		Wait
	};

	int Table(std::vector<Option> options, int columns, bool selectByRow, int* hover, int user, Rectangle area, float rowHeight = fontHeight);
	int List(std::vector<Option> options, int* hover, int user, Rectangle area, float rowHeight = fontHeight);
	int Text(std::string* str, int user, Rectangle area);
	int Motion(std::string* str, int user, Rectangle area);
	int WaitForController(int* input, int user, Rectangle area);
	int WaitForInput(int* input, int user, Rectangle area);
	int ColorPicker(sf::Color* color, int user, Rectangle area);

	void renderText(std::string str, std::string font, sf::Color color, Rectangle area, int align);
	void renderPlayer(Player player, Rectangle capture, Rectangle area);
};

#endif