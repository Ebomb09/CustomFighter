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

	struct Config : public std::vector<Option> {

		/* Data */
		int						data_Columns 		= 1;
		std::string				data_Header			= "";
		bool					data_GroupByRow		= false;

		/* Draw */
		Rectangle 				draw_Area;
		float 					draw_RowHeight 		= fontHeight;
	};

	void getCell(Config& conf, int index, int user, int* outRow = NULL, int* outColumn = NULL, Rectangle* outRectangle = NULL);

	int DoControls(Config& conf, int user, int* hover, bool doControl = true);
	int Table(Config& conf, int user, int* hover, bool doControl = true);
	int Text(Config& conf, int user, std::string* str, bool doControl = true);
	int Motion(Config& conf, int user, std::string* str, bool doControl = true);
	int WaitForController(Config& conf, int user, int* input, bool doControl = true);
	int WaitForInput(Config& conf, int user, int* input, bool doControl = true);
	int ColorPicker(Config& conf, int user, sf::Color* color, bool doControl = true);

	void renderText(std::string str, std::string font, sf::Color color, Rectangle area, int align);
	void renderPlayer(Player player, Rectangle capture, Rectangle area);
};

#endif