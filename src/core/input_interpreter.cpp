#include "input_interpreter.h"

#include "render_instance.h"

InputInterpreter g::input = InputInterpreter();

InputInterpreter::InputInterpreter() {

	for(int i = 0; i < sf::Keyboard::Key::KeyCount; i ++) {
		keyPressed[i] = false;
		keyHeld[i] = false;
		keyReleased[i] = false;
	}

	for(int i = 0; i < sf::Mouse::Button::ButtonCount; i ++) {
		mousePressed[i] = false;
		mouseHeld[i] = false;
		mouseReleased[i] = false;		
	}
}

void InputInterpreter::prepEvents() {

	// Reset pressed / released status
	for(int i = 0; i < sf::Keyboard::Key::KeyCount; i ++) {
		keyPressed[i] = false;
		keyReleased[i] = false;		
	}

	for(int i = 0; i < sf::Mouse::Button::ButtonCount; i ++) {
		mousePressed[i] = false;
		mouseReleased[i] = false;		
	}
	mouseScroll = 0;
	mouseMove = {0, 0};
}

void InputInterpreter::pollEvents() {
	prepEvents();

	sf::Event event;

	while(g::video.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        if(!ImGui::GetIO().WantCaptureKeyboard && !ImGui::GetIO().WantCaptureMouse)
            processEvent(event);		
	}
}

void InputInterpreter::processEvent(const sf::Event& event) {

	if(event.type == sf::Event::Closed) {
		g::video.close();
	}

	if(event.type == sf::Event::KeyPressed) {
		keyPressed[event.key.code] = true;
		keyHeld[event.key.code] = true;			
	}

	if(event.type == sf::Event::KeyReleased) {
		keyReleased[event.key.code] = true;
		keyHeld[event.key.code] = false;			
	}	

	if(event.type == sf::Event::MouseButtonPressed) {
		mousePressed[event.mouseButton.button] = true;
		mouseHeld[event.mouseButton.button] = true;			
	}

	if(event.type == sf::Event::MouseButtonReleased) {
		mouseReleased[event.mouseButton.button] = true;
		mouseHeld[event.mouseButton.button] = false;			
	}

	if(event.type == sf::Event::MouseMoved){
		mouseLastPosition = mousePosition;
		mousePosition = {(float)event.mouseMove.x, (float)event.mouseMove.y};
		mouseMove = {mousePosition.x - mouseLastPosition.x, mousePosition.y - mouseLastPosition.y};
	}

	if(event.type == sf::Event::MouseWheelScrolled)
		mouseScroll = event.mouseWheelScroll.delta;
}