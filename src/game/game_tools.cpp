#include "game_tools.h"

#include "core/render_instance.h"
#include "core/save.h"

using std::vector, std::string;

void setCamera(Player* players, int count) {
	vector<Player> vec;

	for(int i = 0; i < count; i ++)
		vec.push_back(players[i]);

	setCamera(vec);
}

void setCamera(std::vector<Player> players) {
	float pos = 0.f;
	int n = 0;

	for(int i = 0; i < players.size(); i ++) {

		if(players[i].taggedIn(players)) {
			pos += players[i].state.position.x;
			n ++;
		}
	}

	// Smooth motion towards players
    g::video.camera.x = g::video.camera.x + (((pos / n) - (g::video.camera.w / 2.f)) - g::video.camera.x) * 0.10f;
    g::video.camera.y = StageBounds.y - StageBounds.h + g::video.camera.h;	

    // Clamp camera within stage bounds
    g::video.camera.x = std::clamp(g::video.camera.x, StageBounds.x, StageBounds.x + StageBounds.w - g::video.camera.w);
}

void drawHealthBars(vector<Player> players) {
	float health[2] = {0.f, 0.f};

	for(auto& ply : players) {

		if(ply.team == 0 && ply.taggedIn(players))
			health[0] = ply.state.health;

		if(ply.team == 1 && ply.taggedIn(players))
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

void drawHealthBars(Player* players, int count) {
	vector<Player> vec;

	for(int i = 0; i < count; i ++)
		vec.push_back(players[i]);

	drawHealthBars(vec);
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
	text.setColor(sf::Color::White);
	g::video.draw(text);

	text.setString(rString);
	text.setFont(*g::save.getFont("fight"));
	text.setCharacterSize(32);
	text.setPosition(center + Vector2(32, 0));
	text.setColor(sf::Color::White);
	g::video.draw(text);	
}

void drawStage(int index) {
	sf::Texture* tex = g::save.getStage(index);

	sf::Vertex v[4] = {
		sf::Vertex(Vector2(StageBounds.x, StageBounds.y), 									Vector2(0, 0)),
		sf::Vertex(Vector2(StageBounds.x + StageBounds.w, StageBounds.y), 					Vector2(tex->getSize().x, 0)),
		sf::Vertex(Vector2(StageBounds.x + StageBounds.w, StageBounds.y - StageBounds.h),	Vector2(tex->getSize().x, tex->getSize().y)),	
		sf::Vertex(Vector2(StageBounds.x, StageBounds.y - StageBounds.h), 					Vector2(0, tex->getSize().y)),
	};

	for(int i = 0; i < 4; i ++) 
		v[i].position = g::video.camera.getScreen(v[i].position);

	sf::RenderStates states;
	states.texture = tex;

	g::video.draw(v, 4, sf::PrimitiveType::Quads, states);
}