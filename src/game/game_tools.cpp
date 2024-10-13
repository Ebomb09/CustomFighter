#include "game_tools.h"

#include "core/video.h"
#include "core/save.h"

using std::vector, std::string;

void setCamera(Player* players, int count) {
	vector<Player> vec;

	for(int i = 0; i < count; i ++)
		vec.push_back(players[i]);

	setCamera(vec);
}

void setCamera(std::vector<Player> players) {
	Vector2 min = {1000, 1000};
	Vector2 max = {-1000, -1000};
	Vector2 pos = 0.f;
	int n = 0;

	for(int i = 0; i < players.size(); i ++) {

		if(players[i].getTaggedIn(players)) {
			Rectangle area = players[i].getRealBoundingBox();
			area.x -= 8.f;
			area.y += 48.f;
			area.w += 16.f;
			area.h += 50.f;

			pos += {area.x + area.w / 2, area.y - area.h / 2};
			n ++;

			min.x = std::min(min.x, area.x);
			min.y = std::min(min.y, area.y - area.h);
			max.x = std::max(max.x, area.x + area.w);
			max.y = std::max(max.y, area.y);
		}
	}

	if(n == 0)
		n = 1;

	// Smooth motion towards players
	Vector2 center = {g::video.camera.x + g::video.camera.w / 2.f, g::video.camera.y - g::video.camera.h / 2.f};

    center.x += ((pos.x / n) - center.x) * 0.05f;
    center.y += ((pos.y / n) - center.y) * 0.05f;

	// Smooth resize camera
	Vector2 size = {max.x - min.x, max.y - min.y};
	float delta1 = size.y * g::video.camera.w / g::video.camera.h;
	float delta2 = size.x * g::video.camera.h / g::video.camera.w;

	if(delta1 < size.x) size.y = delta2;
	else size.x = delta1;
	
	g::video.camera.w += (size.x - g::video.camera.w) * 0.01f;
	g::video.camera.h += (size.y - g::video.camera.h) * 0.01f;

	// Recenter camera
	g::video.camera.x = center.x - g::video.camera.w / 2.f;
	g::video.camera.y = center.y + g::video.camera.h / 2.f;

    // Clamp camera within stage bounds
    g::video.camera.x = std::clamp(g::video.camera.x, StageBounds.x, StageBounds.x + StageBounds.w - g::video.camera.w);
    g::video.camera.y = std::clamp(g::video.camera.y, StageBounds.y - StageBounds.h + g::video.camera.h, StageBounds.y);		
}

void drawHealthBars(Player* players, int count) {
	vector<Player> vec;

	for(int i = 0; i < count; i ++)
		vec.push_back(players[i]);

	drawHealthBars(vec);
}

void drawHealthBars(vector<Player> players) {
	float health[2] = {0.f, 0.f};

	for(auto& ply : players) {

		if(ply.team == 0 && ply.getTaggedIn(players))
			health[0] = ply.state.health;

		if(ply.team == 1 && ply.getTaggedIn(players))
			health[1] = ply.state.health;		
	}

    float width = (g::video.getSize().x - 32 * 3) / 2.f;
    sf::RectangleShape outline, fill;

    outline.setOutlineColor(sf::Color::White);
    outline.setOutlineThickness(2);
    outline.setFillColor(sf::Color::Transparent);
    fill.setFillColor(sf::Color::Red);

    outline.setSize({width, 32});

    outline.setPosition(32, 32);
    fill.setPosition(32 + width * (1. - health[0] / 100.), 32);
    fill.setSize({width * players[0].state.health / 100, 32});
    g::video.draw(fill);
    g::video.draw(outline);

    outline.setPosition(64 + width, 32);
    fill.setPosition(64 + width, 32);
    fill.setSize({width * health[1] / 100, 32});
    g::video.draw(fill);
    g::video.draw(outline);
}

void drawRoundTokens(int lWin, int rWin, int winMax) {
	Vector2 center = {g::video.getSize().x / 2.f, 64};

	string lString = "";
	string rString = "";

	for(int i = 0; i < winMax; i ++) {

		// right -> left
		if(i < lWin)
			lString = 'X' + lString;
		else
			lString = 'O' + lString;		
			
		// left -> right
		if(i < rWin)
			rString = rString + 'X';
		else
			rString = rString + 'O';						
	}

	sf::Text text;
	text.setString(lString);
	text.setFont(*g::save.getFont("fight"));
	text.setCharacterSize(32);
	text.setPosition(center + Vector2(-32 - text.getLocalBounds().width, 0));
	text.setFillColor(sf::Color::White);
	g::video.draw(text);

	text.setString(rString);
	text.setFont(*g::save.getFont("fight"));
	text.setCharacterSize(32);
	text.setPosition(center + Vector2(32, 0));
	text.setFillColor(sf::Color::White);
	g::video.draw(text);	
}