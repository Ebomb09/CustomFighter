#include "local_game.h"

#include "core/input_interpreter.h"
#include "core/render_instance.h"

void drawHealthBars(Player& p1, Player& p2) {
    float width = (g::video.window.getSize().x - 32 * 3) / 2.f;        
    sf::RectangleShape outline, fill;

    outline.setOutlineColor(sf::Color::White);
    outline.setOutlineThickness(2);
    outline.setFillColor(sf::Color::Transparent);
    fill.setFillColor(sf::Color::Red);

    outline.setSize({width, 32});

    outline.setPosition(32, 32);
    fill.setPosition(32 + width * (1. - p1.state.health / 100.), 32);
    fill.setSize({width * p1.state.health / 100, 32});
    g::video.window.draw(fill);
    g::video.window.draw(outline);

    outline.setPosition(64 + width, 32);
    fill.setPosition(64 + width, 32);
    fill.setSize({width * p2.state.health / 100, 32});
    g::video.window.draw(fill);        
    g::video.window.draw(outline);    
}

bool LocalGame::run(Player::Config p1, Player::Config p2) {
	Player players[2];

	players[0].gameIndex = 0;
	players[0].seatIndex = 0;
	players[0].config = p1;
	players[0].state.target = 1;
	players[0].state.position.x = -75;

	players[1].gameIndex = 1;
	players[1].seatIndex = 1;
	players[1].config = p2;	
	players[1].state.target = 0;
	players[1].state.position.x = 75;	

    while(g::video.window.isOpen()) {
        g::input.prepEvents();

        sf::Event event;
        while(g::video.window.pollEvent(event)) {
            g::input.processEvent(event);
        }

        if(g::input.windowClose)
            g::video.window.close();

        g::video.window.clear();

        // Camera
        g::video.camera.x = (players[0].state.position.x + players[1].state.position.x) / 2. - g::video.camera.w / 2.;
        g::video.camera.y = g::video.camera.h * 0.75;

        drawHealthBars(players[0], players[1]);
        
        vector<Player> old = {players[0], players[1]};
        players[0].advanceFrame(players[0].readInput(), old);
        players[1].advanceFrame(players[1].readInput(), old);
		

        players[0].draw();
        players[1].draw();

        g::video.window.display();
    }
    return true;
}