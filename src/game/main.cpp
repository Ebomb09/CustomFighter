#include "core/input_interpreter.h"
#include "core/render_instance.h"

#include "player.h"

struct GameState {
    Player::State p1;
    Player::State p2;
};

void drawHealthBars(RenderInstance& ri, Player& p1, Player& p2) {
    float width = (ri.window.getSize().x - 32 * 3) / 2;        
    sf::RectangleShape outline, fill;

    outline.setOutlineColor(sf::Color::White);
    outline.setOutlineThickness(2);
    outline.setFillColor(sf::Color::Transparent);
    fill.setFillColor(sf::Color::Red);

    outline.setSize({width, 32});

    outline.setPosition(32, 32);
    fill.setPosition(32 + width * (1. - p1.state.health / 100.), 32);
    fill.setSize({width * p1.state.health / 100, 32});
    ri.window.draw(fill);
    ri.window.draw(outline);

    outline.setPosition(64 + width, 32);
    fill.setPosition(64 + width, 32);
    fill.setSize({width * p2.state.health / 100, 32});
    ri.window.draw(fill);        
    ri.window.draw(outline);    
}

int main(int argc, char* argv[]) {
    g::video.init(1024, 768, "Fighting Room Test");
    g::video.camera.x = -120;
    g::video.camera.y = 120;
    g::video.camera.w /= 4;
    g::video.camera.h /= 4;

    Player p[2];

    p[0].config.opponent = &p[1];

    p[0].config.clothes = {
        "realistic",
        "shirt"
    };

    p[0].config.button = {
        sf::Keyboard::W,   
        sf::Keyboard::S,         
        sf::Keyboard::A,
        sf::Keyboard::D,  
        sf::Keyboard::U,   
        sf::Keyboard::J,   
        sf::Keyboard::I, 
        sf::Keyboard::O,
        sf::Keyboard::Semicolon         
    };

    p[0].config.move[Move::Stand] = "stand";
    p[0].config.move[Move::StandBlock] = "standBlock";
    p[0].config.move[Move::StandCombo] = "standCombo";

    p[0].config.move[Move::WalkBackwards] = "walkBackwards";
    p[0].config.move[Move::WalkForwards] = "walkForwards";

    p[0].config.move[Move::Crouch] = "crouch";
    p[0].config.move[Move::CrouchBlock] = "crouchBlock";
    p[0].config.move[Move::CrouchCombo] = "crouchCombo";

    p[0].config.move[Move::Jump] = "jump";
    p[0].config.move[Move::JumpForwards] = "jumpForwards";
    p[0].config.move[Move::JumpBackwards] = "jumpBackwards";

    p[0].config.move[Move::JumpCombo] = "jumpCombo";

    p[0].config.move[Move::KnockDown] = "knockdown";
    p[0].config.move[Move::GetUp] = "getup";   

    p[0].config.move[Move::Custom00] = "punch";
    p[0].config.motion[Move::Custom00] = "A";

    p[0].config.move[Move::Custom01] = "doubleKick";
    p[0].config.motion[Move::Custom01] = "4B";

    p[0].config.move[Move::Custom02] = "highPunch";
    p[0].config.motion[Move::Custom02] = "623C";

    p[0].config.move[Move::Custom03] = "ankleKick";
    p[0].config.motion[Move::Custom03] = "1B";

    p[0].config.move[Move::Custom04] = "lowDoublePunch";
    p[0].config.motion[Move::Custom04] = "C";

    p[0].config.move[Move::Custom05] = "crouchKick";
    p[0].config.motion[Move::Custom05] = "D";

    p[1].config = p[0].config;

    p[1].config.opponent = &p[0];

    p[1].config.button = {
        sf::Keyboard::Up,  
        sf::Keyboard::Down,            
        sf::Keyboard::Left,
        sf::Keyboard::Right,        
        sf::Keyboard::Numpad4, 
        sf::Keyboard::Numpad1, 
        sf::Keyboard::Numpad5, 
        sf::Keyboard::Numpad6, 
        sf::Keyboard::Enter
    };

    int gameFrame = 0;

    GameState gs;

    while(g::video.window.isOpen()) {
        g::input.prepEvents();

        sf::Event event;
        while(g::video.window.pollEvent(event)) {
            g::input.processEvent(event);
        }

        if(g::input.windowClose)
            g::video.window.close();

        g::video.window.clear();

        // Rollback test

        if(g::input.keyPressed[sf::Keyboard::Space])
            gs = {p[0].state, p[1].state};

        if(g::input.keyPressed[sf::Keyboard::Enter]) {
            p[0].state = gs.p1;
            p[1].state = gs.p2;
        }

        // Camera
        g::video.camera.x = (p[0].state.position.x + p[1].state.position.x) / 2. - g::video.camera.w / 2.;
        g::video.camera.y = g::video.camera.h * 0.75;

        drawHealthBars(g::video, p[0], p[1]);

        for(Rectangle box : p[0].getHitBoxes()) {
            box = g::video.camera.getScreen(box);
            sf::RectangleShape rect = box;
            rect.setFillColor(sf::Color::Red);
            g::video.window.draw(rect);
        }

        for(Rectangle box : p[1].getHitBoxes()) {
            box = g::video.camera.getScreen(box);
            sf::RectangleShape rect = box;
            rect.setFillColor(sf::Color::Red);
            g::video.window.draw(rect);
        }

        for(Rectangle box : p[0].getHurtBoxes()) {
            box = g::video.camera.getScreen(box);
            sf::RectangleShape rect = box;
            rect.setFillColor(sf::Color::Yellow);
            g::video.window.draw(rect);
        }

        for(Rectangle box : p[1].getHurtBoxes()) {
            box = g::video.camera.getScreen(box);
            sf::RectangleShape rect = box;
            rect.setFillColor(sf::Color::Yellow);
            g::video.window.draw(rect);
        }

    	p[0].advanceFrame(p[0].readInput());
    	p[1].advanceFrame(p[1].readInput());

        gameFrame ++;
        
    	p[0].draw();
    	p[1].draw();

    	g::video.window.display();
    }

	return 0;
}