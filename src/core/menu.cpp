#include "menu.h"

#include "button.h"
#include "input_interpreter.h"
#include "render_instance.h"
#include "save.h"

static void cycleIndex(std::vector<Menu::Option> options, int* index, int quantity) {
	int beg = *index;

	do {
		(*index) += quantity;

		// Maximum quantity movement first then try checking closer elements
		if(quantity > 0)
			quantity = 1;

		else if(quantity < 0)
			quantity = -1;

		// Safe check index
		if(*index < 0)
			(*index) = options.size() - 1;
		
		else if(*index >= options.size()) 
			(*index) = 0;		

		// Cyclical check, if we loop back on starting point
		if(*index == beg)
			return;

	}while(options[*index].name == "");
}

static void renderText(string str, string font, sf::Color color, Rectangle area, int align) {

	sf::Text text;
	text.setFont(*g::save.getFont(font));
	text.setCharacterSize(Menu::fontHeight);
	text.setColor(color);	
	text.setString(str);

    // Ensure the text isn't overlapping into the next column
    float testSize = text.getCharacterSize();

    while(text.getLocalBounds().width > area.w || text.getLocalBounds().height > area.h) {
    	text.setCharacterSize(testSize);
    	testSize --;
    }

    // Calculate free space within cell
    Vector2 freeSpace{
    	area.w - text.getLocalBounds().width,
    	area.h - text.getLocalBounds().height
    };

    // Align text
    if(align == 0) {
    	text.setPosition({area.x + freeSpace.x / 2, area.y + freeSpace.y});

    }else if(align == 1){
    	text.setPosition({area.x + freeSpace.x, area.y + freeSpace.y});

    }else {
    	text.setPosition({area.x, area.y + freeSpace.y});
    }

	g::video.draw(text);
}

int Menu::Table(std::vector<Option> options, int columns, bool selectByRow, int* hover, int user, Rectangle area) {
	int status = Wait;

	Button::Config b = g::save.getButtonConfig(user);

	// Move cursor
	if(g::input.keyPressed[b.Up]) 
		cycleIndex(options, hover, -columns);

	if(g::input.keyPressed[b.Down]) 
		cycleIndex(options, hover, columns);

	if(selectByRow) {

		// Ensure only first column is selected
		if(*hover % columns != 0) 
			*hover -= *hover % columns;

	}else {

		if(g::input.keyPressed[b.Left] && !selectByRow) 
			cycleIndex(options, hover, -1);

		if(g::input.keyPressed[b.Right] && !selectByRow) 
			cycleIndex(options, hover, 1);		
	}

	// Safe check index, can be bad on the first table call
	if(*hover < 0)
		(*hover) = options.size() - 1;
	
	else if(*hover >= options.size()) 
		(*hover) = 0;	

	if(g::input.keyPressed[b.B]){
		status = Accept;

	}else if(g::input.keyPressed[b.D]) {
		status = Decline;
	}

	// Draw Properties
	Vector2 pos = Vector2(area.x, area.y);

	int 	rows 				= options.size() / columns;
	int 	selectedRow 		= (*hover) / columns;
	float 	distanceScrolled 	= fontHeight * (selectedRow + 1);
	float	bottomRow			= rows * fontHeight;

    // Set view to scrolled distance
	float scroll = 0;

	// If last row extends past the maximum area then activate scroll
	if(bottomRow > area.h) {

	    if(distanceScrolled > bottomRow - area.h / 2) {
	        scroll = bottomRow - area.h;

	    }else if(distanceScrolled > area.h / 2) {
	        scroll = distanceScrolled - area.h / 2;
	    }		
	}

	sf::View view({
		area.x, 
		area.y + scroll, 
		area.w, 
		area.h
	});

	// View port is relative of screen coordinates
	view.setViewport({
		area.x / g::video.camera.screen_w, 
		area.y / g::video.camera.screen_h, 
		area.w / g::video.camera.screen_w, 
		area.h / g::video.camera.screen_h
	});

	g::video.setView(view);

	// Draw on screen
	for(int i = 0; i < options.size(); i += columns) {

		for(int j = 0; j < columns && i + j < options.size(); j ++) {
			sf::Color color = sf::Color::White;

			if(selectByRow) {

				if(*hover == i)
					color = sf::Color::Yellow;

			}else {

				if(*hover == i + j)
					color = sf::Color::Yellow;
			}

	        renderText(options[i+j].name, options[i+j].font, color, {pos.x + j * (area.w / columns), pos.y, area.w / columns, fontHeight}, 0);
		}
        pos.y += fontHeight;
	}

	g::video.setView(g::video.getDefaultView());

	return status;
}

int Menu::List(std::vector<Option> options, int* hover, int user, Rectangle area) {
	return Table(options, 1, true, hover, user, area);
}

static int keyboardData[4];

int Menu::Text(std::string* str, int user, Rectangle area) {

	renderText(*str, "Anton-Regular", sf::Color::White, {area.x, area.y, area.w, fontHeight}, -1);

	// Adjust remaining area for the keyboard
	area.y += fontHeight*2;
	area.h -= fontHeight*2;

	enum {
		Delete 	= -1,
		Enter	= -2,
		Cancel	= -3
	};

	vector<Option> options;
	options.push_back({"Q", 'Q'});
	options.push_back({"W", 'W'});
	options.push_back({"E", 'E'});
	options.push_back({"R", 'R'});
	options.push_back({"T", 'T'});
	options.push_back({"Y", 'Y'});
	options.push_back({"U", 'U'});
	options.push_back({"I", 'I'});
	options.push_back({"O", 'O'});
	options.push_back({"P", 'P'});

	options.push_back({"A", 'A'});
	options.push_back({"S", 'S'});
	options.push_back({"D", 'D'});
	options.push_back({"F", 'F'});
	options.push_back({"G", 'G'});
	options.push_back({"H", 'H'});
	options.push_back({"J", 'J'});
	options.push_back({"K", 'K'});
	options.push_back({"L", 'L'});
	options.push_back({"Delete", Delete});

	options.push_back({"Z", 'A'});
	options.push_back({"X", 'S'});
	options.push_back({"C", 'D'});
	options.push_back({"V", 'F'});
	options.push_back({"B", 'G'});
	options.push_back({"N", 'H'});
	options.push_back({"M", 'J'});
	options.push_back({"K", 'K'});
	options.push_back({"Space", ' '});
	options.push_back({"Enter", Enter});

	options.push_back({"Cancel", Cancel});

	int res = Table(options, 10, false, &keyboardData[user], user, area);

	if(res == Accept) {

		if(options[keyboardData[user]].id == Enter) {
			return Accept;

		}else if(options[keyboardData[user]].id == Delete) {

			if((*str).size() > 0) 
				(*str).pop_back();			

		}else if(options[keyboardData[user]].id == Cancel) {
			return Decline;

		}else {
			(*str) += (char)options[keyboardData[user]].id;
		}
	}

	return Wait;
}

int Menu::Motion(std::string* str, int user, Rectangle area) {
	Button::Config b = g::save.getButtonConfig(user);

	// Draw the header
	sf::RectangleShape rect({area.w, fontHeight});
	rect.setPosition({area.x, area.y});
	rect.setFillColor(sf::Color(128, 128, 128));
	g::video.draw(rect);

	if(str->size() == 0) {
		renderText("Input Motion...", "Anton-Regular", sf::Color::Black, {area.x, area.y, area.w, fontHeight}, 0);

	}else {
		renderText(*str, "fight", sf::Color::White, {area.x, area.y, area.w, fontHeight}, 0);
	}

	// Create a test player to get SOCD and motion inputs
	Player test;
	test.seatIndex = user;
	test.state.button[0] = test.readInput();

	string motion = "";

	// Key pressed / released and someother key is also being concurrently held
	if((g::input.keyPressed[b.Up] || g::input.keyPressed[b.Down] ||
		g::input.keyPressed[b.Left] || g::input.keyPressed[b.Right] ||
		g::input.keyReleased[b.Up] || g::input.keyReleased[b.Down] ||
		g::input.keyReleased[b.Left] || g::input.keyReleased[b.Right]) &&

		(g::input.keyHeld[b.Up] || g::input.keyHeld[b.Down] ||
		g::input.keyHeld[b.Left] || g::input.keyHeld[b.Right])
		) {

	    Vector2 socd = test.getSOCD();
	    motion += ('5' + (int)socd.x + (int)socd.y * 3);
	}

    if(g::input.keyPressed[b.A]) {

    	if(g::input.keyHeld[b.B] || g::input.keyHeld[b.C] || g::input.keyHeld[b.D]) {
        	motion += "+A"; 
    	}else {
        	motion += "A";    		
    	}
    }

    if(g::input.keyPressed[b.B]) {

    	if(g::input.keyHeld[b.A] || g::input.keyHeld[b.C] || g::input.keyHeld[b.D]) {
        	motion += "+B"; 
    	}else {
        	motion += "B";    		
    	}
    }

    if(g::input.keyPressed[b.C]) {

    	if(g::input.keyHeld[b.A] || g::input.keyHeld[b.B] || g::input.keyHeld[b.D]) {
        	motion += "+C"; 
    	}else {
        	motion += "C";
    	}
    }

    if(g::input.keyPressed[b.D]) {

    	if(g::input.keyHeld[b.A] || g::input.keyHeld[b.B] || g::input.keyHeld[b.C]) {
        	motion += "+D";
    	}else {
        	motion += "D";
    	}
    }

    if(motion.size() > 0) {
    	(*str) += motion;
    }

	if(str->size() > 0 && (g::input.keyReleased[b.A] || g::input.keyReleased[b.B] || g::input.keyReleased[b.C] || g::input.keyReleased[b.D]))
		return Accept;

	return Wait;
}