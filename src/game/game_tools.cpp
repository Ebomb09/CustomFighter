#include "game_tools.h"

#include "core/render_instance.h"
#include "core/save.h"

using std::vector;

void setCamera(Player* players, int count) {
	vector<Player> vec;

	for(int i = 0; i < count; i ++)
		vec.push_back(players[i]);

	setCamera(vec);
}

void setCamera(std::vector<Player> players) {
    g::video.camera.x = (players[0].state.position.x + players[1].state.position.x) / 2. - g::video.camera.w / 2.;
    g::video.camera.y = StageBounds.y - StageBounds.h + g::video.camera.h;	

    // Clamp camera within stage bounds
    g::video.camera.x = std::clamp(g::video.camera.x, StageBounds.x, StageBounds.x + StageBounds.w - g::video.camera.w);
}

void drawHealthBars(vector<Player> players) {

	if(players.size() != 2)
		return;

    float width = (g::video.getSize().x - 32 * 3) / 2.f;
    sf::RectangleShape outline, fill;

    outline.setOutlineColor(sf::Color::White);
    outline.setOutlineThickness(2);
    outline.setFillColor(sf::Color::Transparent);
    fill.setFillColor(sf::Color::Red);

    outline.setSize({width, 32});

    outline.setPosition(32, 32);
    fill.setPosition(32 + width * (1. - players[0].state.health / 100.), 32);
    fill.setSize({width * players[0].state.health / 100, 32});
    g::video.draw(fill);
    g::video.draw(outline);

    outline.setPosition(64 + width, 32);
    fill.setPosition(64 + width, 32);
    fill.setSize({width * players[1].state.health / 100, 32});
    g::video.draw(fill);        
    g::video.draw(outline);    
}

void drawHealthBars(Player* players, int count) {
	vector<Player> vec;

	for(int i = 0; i < count; i ++)
		vec.push_back(players[i]);

	drawHealthBars(vec);
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