#include "core/input_interpreter.h"
#include "core/render_instance.h"

#include "player.h"

void drawHealthBars(RenderInstance& ri, Player::State& p1, Player::State& p2) {
    float width = (ri.window.getSize().x - 32 * 3) / 2;        
    sf::RectangleShape outline, fill;

    outline.setOutlineColor(sf::Color::White);
    outline.setOutlineThickness(2);
    outline.setFillColor(sf::Color::Transparent);
    fill.setFillColor(sf::Color::Red);

    outline.setSize({width, 32});

    outline.setPosition(32, 32);
    fill.setPosition(32 + width * (1. - p1.health / 100.), 32);
    fill.setSize({width * p1.health / 100, 32});
    ri.window.draw(fill);        
    ri.window.draw(outline);

    outline.setPosition(64 + width, 32);
    fill.setPosition(64 + width, 32);
    fill.setSize({width * p2.health / 100, 32});
    ri.window.draw(fill);        
    ri.window.draw(outline);    
}

int main(int argc, char* argv[]) {
    g::video.init(1024, 768, "Fighting Room Test");
    g::video.camera.x = -120;
    g::video.camera.y = 120;
    g::video.camera.w /= 4;
    g::video.camera.h /= 4;

    Player p1;
    Player p2;

    p1.config.button[Button::Left] = sf::Keyboard::A;
    p1.config.button[Button::Right] = sf::Keyboard::D;
    p1.config.button[Button::Up] = sf::Keyboard::W;   
    p1.config.button[Button::Down] = sf::Keyboard::S;   
    p1.config.button[Button::A] = sf::Keyboard::U;   
    p1.config.button[Button::B] = sf::Keyboard::J;   
    p1.config.button[Button::C] = sf::Keyboard::I; 
    p1.config.button[Button::D] = sf::Keyboard::O; 
    p1.config.button[Button::Taunt] = sf::Keyboard::Semicolon; 

    p1.config.move[Move::Stand] = "stand";
    p1.config.move[Move::StandBlock] = "standBlock";
    p1.config.move[Move::StandCombo] = "standCombo";

    p1.config.move[Move::Crouch] = "crouch";
    p1.config.move[Move::CrouchBlock] = "crouchBlock";
    p1.config.move[Move::CrouchCombo] = "crouchCombo";

    p1.config.move[Move::Jump] = "jump";
    p1.config.move[Move::JumpCombo] = "jumpCombo";

    p1.config.move[Move::KnockDown] = "knockdown";
    p1.config.move[Move::GetUp] = "getup";   

    p2.config.button[Button::Left] = sf::Keyboard::Left;
    p2.config.button[Button::Right] = sf::Keyboard::Right;
    p2.config.button[Button::Up] = sf::Keyboard::Up;   
    p2.config.button[Button::Down] = sf::Keyboard::Down;   
    p2.config.button[Button::A] = sf::Keyboard::Numpad4;   
    p2.config.button[Button::B] = sf::Keyboard::Numpad1;   
    p2.config.button[Button::C] = sf::Keyboard::Numpad5; 
    p2.config.button[Button::D] = sf::Keyboard::Numpad6; 
    p2.config.button[Button::Taunt] = sf::Keyboard::Enter;

    p2.config.move[Move::Custom00] = "punch";
    p2.config.motion[Move::Custom00] = "A";

    p2.config.move[Move::Custom01] = "doubleKick";
    p2.config.motion[Move::Custom01] = "4B";

    p2.config.move[Move::Custom02] = "highPunch";
    p2.config.motion[Move::Custom02] = "623C";

    p2.config.move[Move::Custom03] = "ankleKick";
    p2.config.motion[Move::Custom03] = "1B";

    p2.config.move[Move::Custom04] = "lowDoublePunch";
    p2.config.motion[Move::Custom04] = "C";

    p2.config.move[Move::Custom05] = "crouchKick";
    p2.config.motion[Move::Custom05] = "D";

    p1.opponent = &p2;
    p2.opponent = &p1;

    p1.getState(0).health = 70;
    p2.getState(0).health = 30;

    p1.getState(0).position.x = -50;
    p2.getState(0).position.x = 50;

    int gameFrame = 0;

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
        g::video.camera.x = (p1.getState(gameFrame).position.x + p2.getState(gameFrame).position.x) / 2. - g::video.camera.w / 2.;
        g::video.camera.y = g::video.camera.h * 0.75;

        drawHealthBars(g::video, p1.getState(gameFrame), p2.getState(gameFrame));

        for(Rectangle box : p1.getState(gameFrame).getHitBoxes()) {
            box = g::video.camera.getScreen(box);
            sf::RectangleShape rect = box;
            rect.setFillColor(sf::Color::Red);
            g::video.window.draw(rect);
        }

        for(Rectangle box : p2.getState(gameFrame).getHitBoxes()) {
            box = g::video.camera.getScreen(box);
            sf::RectangleShape rect = box;
            rect.setFillColor(sf::Color::Red);
            g::video.window.draw(rect);
        }

        for(Rectangle box : p1.getState(gameFrame).getHurtBoxes()) {
            box = g::video.camera.getScreen(box);
            sf::RectangleShape rect = box;
            rect.setFillColor(sf::Color::Yellow);
            g::video.window.draw(rect);
        }

        for(Rectangle box : p2.getState(gameFrame).getHurtBoxes()) {
            box = g::video.camera.getScreen(box);
            sf::RectangleShape rect = box;
            rect.setFillColor(sf::Color::Yellow);
            g::video.window.draw(rect);
        }

        p1.readInput(gameFrame);
        p2.readInput(gameFrame);

    	p1.getNextState(gameFrame);
    	p2.getNextState(gameFrame);

        gameFrame ++;

    	p1.draw(gameFrame);
    	p2.draw(gameFrame);

    	g::video.window.display();

    }

	return 0;
}