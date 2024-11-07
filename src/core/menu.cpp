#include "menu.h"
#include "button.h"
#include "input_interpreter.h"
#include "math.h"
#include "player.h"
#include "video.h"
#include "audio.h"
#include "save.h"

#include <cmath>
#include <utility>
#include <string>
#include <iostream>

using std::vector, std::string;

Menu::Option::Option() {
	type = Type::Empty;
	id = -1;
	data = 0;
}

Menu::Option::Option(int _id, std::string _text, std::string _font, int _align) {
	type = Type::Text;
	id = _id;
	text = new string(_text);
	font = new string(_font);
	align = _align;
}

Menu::Option::Option(int _id, Player _player, Rectangle _capture) {

	if(_capture.w == 0 || _capture.h == 0)
		_capture = _player.getRealBoundingBox();

	type = Type::Player;
	id = _id;
	player = new Player(_player);
	capture = new Rectangle(_capture);
}

Menu::Option::Option(int _id, int _data, std::string _text, std::string _font, int _align) {
	type = Type::Text;
	id = _id;
	data = _data;
	text = new string(_text);
	font = new string(_font);
	align = _align;
}

Menu::Option::Option(int _id, int _data, Player _player, Rectangle _capture) {

	if(_capture.w == 0 || _capture.h == 0)
		_capture = _player.getRealBoundingBox();

	type = Type::Player;
	id = _id;
	data = _data;
	player = new Player(_player);
	capture = new Rectangle(_capture);
}

Menu::Option::~Option() {

	switch(type) {

	case Type::Text:

		if(text) delete text;
		if(font) delete font;
		break;

	case Type::Player:
		if(player) delete player;
		if(capture) delete capture;		
		break;		

	default:
		break;
	}
}

Menu::Option::Option(Option&& move) {
	id = std::move(move.id);
	data = std::move(move.data);
	type = std::exchange(move.type, Type::Empty);

	switch(type) {

	case Type::Text:
		text = std::move(move.text);
		font = std::move(move.font);
		align = std::move(move.align);
		break;

	case Type::Player:
		player = std::move(move.player);
		capture = std::move(move.capture);		
		break;		

	default:
		break;
	}
}

Menu::Option::Option(const Option& copy) {
	id = copy.id;
	data = copy.data;
	type = copy.type;

	switch(type) {

	case Type::Text:
		text = new string(*copy.text);
		font = new string(*copy.font);
		align = copy.align;		
		break;

	case Type::Player:
		player = new Player(*copy.player);
		capture = new Rectangle(*copy.capture);		
		break;		

	default:
		break;
	}
}

static void cycleIndex(const std::vector<Menu::Option>& options, int* index, int quantity) {

	if(!index)
		return;

	int beg = *index;

	do {
		(*index) += quantity;

		// Safe check index
		if(*index < 0)
			(*index) = options.size() + *index;
		
		else if(*index >= options.size()) 
			(*index) = *index - options.size();		

		// Cyclical check, if we loop back on starting point
		if(*index == beg && quantity != 0)
			return;

		// Maximum quantity movement first then try checking closer elements
		if(quantity >= 0)
			quantity = 1;

		else if(quantity < 0)
			quantity = -1;

	}while(options[*index].type == Menu::Option::Type::Empty || options[*index].id < 0);
}

static float scroll[MAX_PLAYERS] {0, 0, 0, 0};
static int items[MAX_PLAYERS] {0, 0, 0, 0};

void Menu::getCell(Menu::Config& conf, int index, int user, int* outRow, int* outColumn, Rectangle* outRectangle) {
	int row = index / conf.data_Columns;
	int column = index % conf.data_Columns;

	if(outRow)
		*outRow = row;

	if(outColumn)
		*outColumn = column;

	Rectangle area = {
		conf.draw_Area.x + column * (conf.draw_Area.w / conf.data_Columns), 
		conf.draw_Area.y + row * conf.draw_RowHeight - scroll[user], 
		conf.draw_Area.w / conf.data_Columns, 
		conf.draw_RowHeight
	};

	if(outRectangle)
		*outRectangle = area;
}  

int Menu::DoControls(Config& conf, int user, int* hover, bool doControl) {

	if(!doControl || user < 0 || !hover) 
		return Wait;

	Button::Config b = g::save.getButtonConfig(user);
	int initial = *hover;

	bool buttonAccept = g::input.pressed(b.index, b.B);
	bool buttonDecline = g::input.pressed(b.index, b.D);

	// Move cursor
	if(g::input.pressed(b.index, b.Up)) {
		cycleIndex(conf, hover, -conf.data_Columns);
		
	}else if(g::input.pressed(b.index, b.Down)) {
		cycleIndex(conf, hover, conf.data_Columns);
	}

	// Mouse controls
	if(Screen::pointInRectangle(g::input.mousePosition, conf.draw_Area)) {

		for(int i = 0; i < conf.size(); i ++) {
			int row, column;
			Rectangle area;
			getCell(conf, i, user, &row, &column, &area);

			if(Screen::pointInRectangle(g::input.mousePosition, area)) {
				*hover = i;

				if(g::input.pressed(MOUSE_INDEX, sf::Mouse::Button::Left))
					buttonAccept = true;
					
				break;
			}
		}
	}

	// Ensure only first column is selected
	if(conf.data_GroupByRow) {

		if(*hover % conf.data_Columns != 0) 
			*hover -= *hover % conf.data_Columns;

	// Can move left or right within table
	}else {

		if(g::input.pressed(b.index, b.Left)) {
			cycleIndex(conf, hover, -1);

		}else if(g::input.pressed(b.index, b.Right)) {
			cycleIndex(conf, hover, 1);
		}
	}

	// Safe check index, can be bad on the first table call
	cycleIndex(conf, hover, 0);

	// Changed the hovered item
	if(*hover != initial) {
		g::audio.playSound(g::save.getSound("cycle"));
	}

	// South button - Accept
	if(buttonAccept){
		g::audio.playSound(g::save.getSound("select"));
		return Accept;

	// East button - Decline
	}else if(buttonDecline) {
		return Decline;
	}
	return Wait;
}

int Menu::Table(Config& conf, int user, int* hover, bool doControl) {
	int status = DoControls(conf, user, hover, doControl);

	// Reality check the area of the table
	if(conf.draw_Area.w <= 4.f || conf.draw_Area.h <= 4.f)
		return status;

	// Draw Properties
	int 	rows 				= conf.size() / conf.data_Columns;
	float	bottomRow			= rows * conf.draw_RowHeight;

	// Check if item count has changed before performing scroll
	if(items[user] != conf.size()) {
		items[user] = conf.size();
		scroll[user] = 0;
	}

    // Set view to scrolled distance
	float desiredScroll = 0;

	// If last row extends past the maximum area then activate scroll
	if(hover) {
		if(bottomRow > conf.draw_Area.h) {
			float selectedRow = 1.f * (*hover) / conf.data_Columns;

			// Override if mouse on screen
			if(doControl && Screen::pointInRectangle(g::input.mousePosition, conf.draw_Area)) 
				selectedRow = (g::input.mousePosition.y - conf.draw_Area.y) / conf.draw_Area.h * rows;

			// Calculate the distance scrolled down table
			float distanceScrolled = conf.draw_RowHeight * (selectedRow + 1);

			// Clamp to bottom
			if(distanceScrolled > bottomRow - conf.draw_Area.h / 2) {
				desiredScroll = bottomRow - conf.draw_Area.h;

			// Clamp to scroll middle
			}else if(distanceScrolled > conf.draw_Area.h / 2) {
				desiredScroll = distanceScrolled - conf.draw_Area.h / 2;
			}		
		}
	}

	// Scroll effect
	if(scroll[user] < desiredScroll)
		scroll[user] = std::min(desiredScroll, scroll[user] + (desiredScroll - scroll[user]) / 10);

	if(scroll[user] > desiredScroll)
		scroll[user] = std::max(desiredScroll, scroll[user] - (scroll[user] - desiredScroll) / 10);

	sf::View view({
		conf.draw_Area.x, 
		conf.draw_Area.y, 
		conf.draw_Area.w, 
		conf.draw_Area.h
	});

	// View port is relative of screen coordinates
	view.setViewport(conf.draw_Area.getRatio(g::video.getSize().x, g::video.getSize().y));

	g::video.setView(view);

	// Draw on screen
	for(int i = 0; i < conf.size(); i ++) {
		int row, column;
		Rectangle area;
		getCell(conf, i, user, &row, &column, &area);

		// Get the text colour
		sf::Color color = sf::Color::White;

		if(conf.data_GroupByRow) {

			if(hover && *hover == row * conf.data_Columns)
				color = sf::Color::Yellow;

		}else {

			if(hover && *hover == i)
				color = sf::Color::Yellow;
		}

		// Render the cell type
		switch(conf[i].type) {

		case Option::Type::Text:
			renderText(*conf[i].text, *conf[i].font, color, area, conf[i].align);
			break;

		case Option::Type::Player: 
			renderPlayer(*conf[i].player, *conf[i].capture, area);
			break;
		
		default:
			break;
		}

		// Outline the cell
		bool drawOutline = false;

		if(conf.data_GroupByRow) {
			drawOutline = (hover && *hover == row * conf.data_Columns);

		}else {
			drawOutline = (hover && *hover == i);
		}

		if(drawOutline) {
			sf::RectangleShape rect = Rectangle{area.x, area.y, area.w, 2.f};
			rect.setFillColor(sf::Color::Yellow);
			g::video.draw(rect);

			rect = Rectangle{area.x, area.y + area.h - 2.f, area.w, 2.f};
			rect.setFillColor(sf::Color::Yellow);
			g::video.draw(rect);
		}
	}

	g::video.setView(g::video.getDefaultView());
	return status;
}

static int keyboardData[MAX_PLAYERS];

int Menu::Text(Config& conf, int user, string* str, bool doControl) {

	// Draw the input text
	renderText(*str, "Anton-Regular", sf::Color::White, {conf.draw_Area.x, conf.draw_Area.y, conf.draw_Area.w, fontHeight}, -1);

	// Adjust remaining area for the keyboard
	conf.draw_Area.y += fontHeight*2;
	conf.draw_Area.h -= fontHeight*2;

	enum {
		Delete 	= -1,
		Enter	= -2,
		Cancel	= -3
	};
	
	// Add alphabet
	conf.data_Columns = 10;

	conf.push_back(Option('Q', "Q"));
	conf.push_back(Option('W', "W"));
	conf.push_back(Option('E', "E"));
	conf.push_back(Option('R', "R"));
	conf.push_back(Option('T', "T"));
	conf.push_back(Option('Y', "Y"));
	conf.push_back(Option('U', "U"));
	conf.push_back(Option('I', "I"));
	conf.push_back(Option('O', "O"));
	conf.push_back(Option('P', "P"));

	conf.push_back(Option('A', "A"));
	conf.push_back(Option('S', "S"));
	conf.push_back(Option('D', "D"));
	conf.push_back(Option('F', "F"));
	conf.push_back(Option('G', "G"));
	conf.push_back(Option('H', "H"));
	conf.push_back(Option('J', "J"));
	conf.push_back(Option('K', "K"));
	conf.push_back(Option('L', "L"));
	conf.push_back(Option(Delete, "Delete"));

	conf.push_back(Option('Z', "Z"));
	conf.push_back(Option('X', "X"));
	conf.push_back(Option('C', "C"));
	conf.push_back(Option('V', "V"));
	conf.push_back(Option('B', "B"));
	conf.push_back(Option('N', "N"));
	conf.push_back(Option('M', "M"));
	conf.push_back(Option('K', "K"));
	conf.push_back(Option(' ', "Space"));
	conf.push_back(Option(Enter, "Enter"));

	conf.push_back(Option(Cancel, "Cancel"));

	int res = Table(conf, user, &keyboardData[user], true);

	if(res == Accept) {

		if(conf[keyboardData[user]].id == Enter) {
			return Accept;

		}else if(conf[keyboardData[user]].id == Delete) {

			if((*str).size() > 0) 
				(*str).pop_back();			

		}else if(conf[keyboardData[user]].id == Cancel) {
			return Decline;

		}else {
			(*str) += (char)conf[keyboardData[user]].id;
		}
	}

	return Wait;
}

static int motionData[MAX_PLAYERS];

int Menu::Motion(Config& conf, int user, string* str, bool doControl) {
	Button::Config b = g::save.getButtonConfig(user);

	// Draw the header
	sf::RectangleShape rect({conf.draw_Area.w, fontHeight});
	rect.setPosition({conf.draw_Area.x, conf.draw_Area.y});
	rect.setFillColor(sf::Color(128, 128, 128));
	g::video.draw(rect);

	if(str->size() == 0) {
		renderText("Input Motion...", "Anton-Regular", sf::Color::Black, {conf.draw_Area.x, conf.draw_Area.y, conf.draw_Area.w, fontHeight}, 0);

	}else {
		renderText(*str, "fight", sf::Color::White, {conf.draw_Area.x, conf.draw_Area.y, conf.draw_Area.w, fontHeight}, 0);
	}

	if(doControl) {
		// Create a test player to get SOCD and motion inputs
		Player test;
		test.seatIndex = user;
		test.state.button[0] = test.readInput();

		// Emulate player input buffer by check press / release
		string motion = "";

		if((g::input.pressed(b.index, b.Up) 	|| g::input.released(b.index, b.Up) ||
			g::input.pressed(b.index, b.Left) 	|| g::input.released(b.index, b.Left) ||
			g::input.pressed(b.index, b.Down) 	|| g::input.released(b.index, b.Down) ||
			g::input.pressed(b.index, b.Right) 	|| g::input.released(b.index, b.Right)) &&
			(g::input.held(b.index, b.Up) ||
			g::input.held(b.index, b.Left) ||
			g::input.held(b.index, b.Down) ||
			g::input.held(b.index, b.Right))) {

			Vector2 socd = test.getSOCD();
			motion += ('5' + (int)socd.x + (int)socd.y * 3);
		}

		// Get button states
		string button = "";

		if(g::input.pressed(b.index, b.A)) button += (button.size() == 0) ? "A" : "+A";
		if(g::input.pressed(b.index, b.B)) button += (button.size() == 0) ? "B" : "+B";
		if(g::input.pressed(b.index, b.C)) button += (button.size() == 0) ? "C" : "+C";
		if(g::input.pressed(b.index, b.D)) button += (button.size() == 0) ? "D" : "+D";

		(*str) += motion + button;

		// Return when no more buttons are being held
		bool held = false;

		for(int i = 0; i < Button::Total; i ++) {

			if(g::input.pressed(b.index, b.button[i])) {
				held = true;
				break;
			}
		}

		// Wait a couple frames before returning
		if(held) {
			motionData[user] = 20;

		}else {
			motionData[user] --;
		}
	}

	if(str->size() > 0 && motionData[user] <= 0)
		return Accept;

	return Wait;
}

int Menu::WaitForController(Config& conf, int user, int* input, bool doControl) {
	Button::Config b = g::save.getButtonConfig(user);

	// Draw the header
	sf::RectangleShape rect({conf.draw_Area.w, fontHeight});
	rect.setPosition({conf.draw_Area.x, conf.draw_Area.y});
	rect.setFillColor(sf::Color(128, 128, 128));
	g::video.draw(rect);

	renderText("Press any button...", "Anton-Regular", sf::Color::Black, {conf.draw_Area.x, conf.draw_Area.y, conf.draw_Area.w, fontHeight}, 0);

	int index = g::input.lastController;

	if(doControl && g::input.pressed(index, g::input.controller[index].lastInput)) {
		*input = index;
		return Accept;
	}
	return Wait;
}

int Menu::WaitForInput(Config& conf, int user, int* input, bool doControl) {
	Button::Config b = g::save.getButtonConfig(user);

	// Draw the header
	sf::RectangleShape rect({conf.draw_Area.w, fontHeight});
	rect.setPosition({conf.draw_Area.x, conf.draw_Area.y});
	rect.setFillColor(sf::Color(128, 128, 128));
	g::video.draw(rect);

	renderText("Press any button...", "Anton-Regular", sf::Color::Black, {conf.draw_Area.x, conf.draw_Area.y, conf.draw_Area.w, fontHeight}, 0);

	int last = g::input.controller[b.index].lastInput;

	if(doControl && g::input.pressed(b.index, last)) {
		*input = last;
		return Accept;
	}
	return Wait;
}

static int colorSelect[MAX_PLAYERS];

int Menu::ColorPicker(Config& conf, int user, sf::Color* color, bool doControl) {
	Button::Config b = g::save.getButtonConfig(user);

	float scroll = 0;

	for(int i = 0; i < 3; i ++) {
		Rectangle box = {conf.draw_Area.x, conf.draw_Area.y + scroll, conf.draw_Area.w, fontHeight};

		sf::Vertex vert[4];
		vert[0].position = {box.x, box.y};
		vert[0].color = *color;		
		vert[1].position = {box.x, box.y + box.h};
		vert[1].color = *color;
		vert[2].position = {box.x + box.w, box.y + box.h};
		vert[2].color = *color;
		vert[3].position = {box.x + box.w, box.y};
		vert[3].color = *color;

		string edit = "";
		int value = 0;

		if(i == 0) {

			if(g::input.held(MOUSE_INDEX, sf::Mouse::Button::Left)) {
				if(Screen::pointInRectangle(g::input.mousePosition, box)) {
					color->r = (g::input.mousePosition.x - box.x) / box.w * 255;
				}
			}

			edit = "Red: " + std::to_string(color->r);
			value = color->r;
			vert[0].color.r = 0;
			vert[1].color.r = 0;
			vert[2].color.r = 255;
			vert[3].color.r = 255;	

		}else if(i == 1){

			if(g::input.held(MOUSE_INDEX, sf::Mouse::Button::Left)) {
				if(Screen::pointInRectangle(g::input.mousePosition, box)) {
					color->g = (g::input.mousePosition.x - box.x) / box.w * 255;
				}
			}

			edit = "Green: " + std::to_string(color->g);	
			value = color->g;					
			vert[0].color.g = 0;
			vert[1].color.g = 0;
			vert[2].color.g = 255;
			vert[3].color.g = 255;	

		}else {

			if(g::input.held(MOUSE_INDEX, sf::Mouse::Button::Left)) {
				if(Screen::pointInRectangle(g::input.mousePosition, box)) {
					color->b = (g::input.mousePosition.x - box.x) / box.w * 255;
				}
			}

			edit = "Blue: " + std::to_string(color->b);
			value = color->b;						
			vert[0].color.b = 0;
			vert[1].color.b = 0;
			vert[2].color.b = 255;
			vert[3].color.b = 255;	
		}

		// Draw the bars
		g::video.draw(vert, 4, sf::PrimitiveType::Quads);

		// Draw the color position
		sf::RectangleShape rect;
		rect.setPosition({box.x + (value / 255.f) * box.w - 1, box.y});
		rect.setSize({2, fontHeight});
		rect.setFillColor(sf::Color::White);
		rect.setOutlineThickness(3);
		rect.setOutlineColor(sf::Color::Black);
		g::video.draw(rect);

		renderText(edit, "Anton-Regular", (colorSelect[user] == i) ? sf::Color::Yellow : sf::Color::Black, box, 0);

		scroll += 64;		
	}

	// Select R,G,B to modify
	if(g::input.pressed(b.index, b.Up))		colorSelect[user] --;
	if(g::input.pressed(b.index, b.Down))	colorSelect[user] ++;
	if(colorSelect[user] < 0)				colorSelect[user] = 2;
	if(colorSelect[user] > 2)				colorSelect[user] = 0;

	// Modify color values
	unsigned char* val = NULL;	
	int adjust = 0;

	if(g::input.held(b.index, b.Left))		adjust = -1;
	if(g::input.held(b.index, b.Right))		adjust = 1;

	switch(colorSelect[user]) {
		case 0: val = &color->r; break;
		case 1: val = &color->g; break;
		case 2: val = &color->b; break;
	}

	if(val) {
		if((adjust < 0 && *val > 0) || (adjust > 0 && *val < 255))
			*val += adjust;
	}

	if(g::input.pressed(b.index, b.B))
		return Accept;

	if(g::input.pressed(b.index, b.D))
		return Decline;

	return Wait;
}

void Menu::renderText(string str, string font, sf::Color color, Rectangle area, int align) {

	sf::Text text;
	text.setFont(*g::save.getFont(font));
	text.setCharacterSize(Menu::fontHeight);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(1);
	text.setFillColor(color);	
	text.setString(str);

	// Test text to determine universal height
	sf::Text test;
	test.setFont(*g::save.getFont(font));
	test.setCharacterSize(Menu::fontHeight);
	test.setOutlineColor(sf::Color::Black);
	test.setOutlineThickness(1);
	test.setFillColor(color);
	test.setString("123456789ABCD+");

    // Ensure the text isn't overlapping into the next column
    while(text.getGlobalBounds().width > area.w || test.getGlobalBounds().height > area.h) {
    	text.setCharacterSize(text.getCharacterSize() - 1);
    	test.setCharacterSize(test.getCharacterSize() - 1);

		// Limit how small the text can actually be
		if(text.getCharacterSize() <= 4)
			return;
	}

    // Calculate free space within cell
    Vector2 freeSpace{
    	area.w - text.getGlobalBounds().width,
    	area.h - text.getGlobalBounds().height
    };

	// Set the origin from the top left pixel of the font
	text.setOrigin(text.getLocalBounds().left, text.getLocalBounds().top);

    // Align text
    if(align == 0) {
    	text.setPosition({area.x + freeSpace.x / 2, area.y + freeSpace.y / 2});

    }else if(align == 1){
    	text.setPosition({area.x + freeSpace.x, area.y + freeSpace.y / 2});

    }else {
    	text.setPosition({area.x, area.y + freeSpace.y / 2});
    }

	g::video.draw(text);
}

static Renderer renderer(TargetMode::Texture);

void Menu::renderPlayer(Player player, Rectangle capture, Rectangle area) {

    renderer.setSize({area.w, area.h});

    if(renderer.reload()) {

    	// Increase padding of capture
	    capture.x -= 16.f;
	    capture.y += 16.f;
	    capture.w += 32.f;
	    capture.h += 32.f;

	    // Scale bounding box to proper aspect ratio
	    Vector2 center = {capture.x + capture.w / 2, capture.y - capture.h / 2};

	    if(capture.w / capture.h < area.w / area.h) {
	        capture.w = capture.h * area.w / area.h;

	    }else {
	        capture.h = capture.w * area.h / area.w;
	    }

		// Set camera to the location of player
		renderer.camera = {
		    center.x - capture.w / 2,
		    center.y + capture.h / 2,
		    capture.w,
		    capture.h
		};

    	renderer.clear();

	    // Clear the area
	    sf::RectangleShape rect;
	    rect.setSize({area.w, area.h});
	    rect.setFillColor(sf::Color(158, 215, 240));
	    renderer.draw(rect);

	    // Draw floor
        rect = renderer.toScreen(Rectangle{
            -1000,
            0,
            2000,
            1000
        });
        rect.setFillColor(sf::Color(2, 80, 158));
        renderer.draw(rect);

	    // Draw player
	    player.draw(&renderer);

	    renderer.display();

	    // Draw the output image
	    sf::RectangleShape out = area;
	    out.setTexture(renderer.getTexture());
	    g::video.draw(out);
    }
}