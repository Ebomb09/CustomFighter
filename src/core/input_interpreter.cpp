#include "input_interpreter.h"
#include "render_instance.h"

using std::string;

InputInterpreter g::input = InputInterpreter();

void InputInterpreter::prepEvents() {

	// Reset pressed / released status
	for(int i = 0; i < MAX_CONTROLLERS; i ++) {

		for(int j = 0; j < MAX_INPUTS; j ++) {
			controller[i].pressed[j] = false;
			controller[i].released[j] = false;				
		}
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
		controller[KEYBOARD_INDEX].held[event.key.code] = true;
		controller[KEYBOARD_INDEX].pressed[event.key.code] = true;

		controller[KEYBOARD_INDEX].lastInput = event.key.code;
		lastController = KEYBOARD_INDEX;
	}

	if(event.type == sf::Event::KeyReleased) {
		controller[KEYBOARD_INDEX].released[event.key.code] = true;
		controller[KEYBOARD_INDEX].held[event.key.code] = false;	
	}	

	if(event.type == sf::Event::MouseButtonPressed) {
		controller[MOUSE_INDEX].pressed[event.mouseButton.button] = true;
		controller[MOUSE_INDEX].held[event.mouseButton.button] = true;	
	}

	if(event.type == sf::Event::MouseButtonReleased) {
		controller[MOUSE_INDEX].released[event.mouseButton.button] = true;
		controller[MOUSE_INDEX].held[event.mouseButton.button] = false;	
	}

	if(event.type == sf::Event::MouseMoved){
		mouseLastPosition = mousePosition;
		mousePosition = {(float)event.mouseMove.x, (float)event.mouseMove.y};
		mouseMove = {mousePosition.x - mouseLastPosition.x, mousePosition.y - mouseLastPosition.y};
	}

	if(event.type == sf::Event::MouseWheelScrolled)
		mouseScroll = event.mouseWheelScroll.delta;

	if(event.type == sf::Event::JoystickButtonPressed) {
		controller[event.joystickButton.joystickId].held[event.joystickButton.button] = true;
		controller[event.joystickButton.joystickId].pressed[event.joystickButton.button] = true;

		controller[event.joystickButton.joystickId].lastInput = event.joystickButton.button;
		lastController = event.joystickButton.joystickId;
	}

	if(event.type == sf::Event::JoystickButtonReleased) {
		controller[event.joystickButton.joystickId].released[event.joystickButton.button] = true;
		controller[event.joystickButton.joystickId].held[event.joystickButton.button] = false;
	}

	// Normalize the axis to emulate physical buttons
	if(event.type == sf::Event::JoystickMoved) {

		for(int i = 0; i < 2; i ++) {
			int bIndex = sf::Joystick::ButtonCount + (int)event.joystickMove.axis * 2 + i;
			bool on = (i == 0) ? event.joystickMove.position > 50 : event.joystickMove.position < -50;

			controller[event.joystickMove.joystickId].pressed[bIndex] = on && !controller[event.joystickMove.joystickId].held[bIndex];
			controller[event.joystickMove.joystickId].released[bIndex] = !on && controller[event.joystickMove.joystickId].held[bIndex];			
			controller[event.joystickMove.joystickId].held[bIndex] = on;

			if(controller[event.joystickMove.joystickId].pressed[bIndex]) {
				controller[event.joystickMove.joystickId].lastInput = bIndex;
				lastController = event.joystickMove.joystickId;	
			}
		}
	}	
}

bool InputInterpreter::pressed(int index, int input) {

	if(index < 0 || index >= MAX_CONTROLLERS)
		return false;

	if(input < 0 || input >= MAX_INPUTS)
		return false;

	return controller[index].pressed[input];
}

bool InputInterpreter::released(int index, int input) {

	if(index < 0 || index >= MAX_CONTROLLERS)
		return false;

	if(input < 0 || input >= MAX_INPUTS)
		return false;

	return controller[index].released[input];
}

bool InputInterpreter::held(int index, int input) {

	if(index < 0 || index >= MAX_CONTROLLERS)
		return false;

	if(input < 0 || input >= MAX_INPUTS)
		return false;

	return controller[index].held[input];
}

string InputInterpreter::controllerName(int index) {

	if(index == KEYBOARD_INDEX)
		return "Keyboard";

	else if(index == MOUSE_INDEX)
		return "Mouse";

	else if(index >= 0 && index < sf::Joystick::Count)
		return sf::Joystick::getIdentification(index).name;

	return "<No Controller Detected>";
}

static const string keysToString[] = {
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"Num0",
	"Num1",
	"Num2",
	"Num3",
	"Num4",
	"Num5",
	"Num6",
	"Num7",
	"Num8",
	"Num9",
	"Escape",
	"LControl",
	"LShift",
	"LAlt",
	"LSystem",
	"RControl",
	"RShift",
	"RAlt",
	"RSystem",
	"Menu",
	"LBracket",
	"RBracket",
	"Semicolon",
	"Comma",
	"Period",
	"Apostrophe",
	"Slash",
	"Backslash",
	"Grave",
	"Equal",
	"Hyphen",
	"Space",
	"Enter",
	"Backspace",
	"Tab",
	"PageUp",
	"PageDown",
	"End",
	"Home",
	"Insert",
	"Delete",
	"Add",
	"Subtract",
	"Multiply",
	"Divide",
	"Left",
	"Right",
	"Up",
	"Down",
	"Numpad0",
	"Numpad1",
	"Numpad2",
	"Numpad3",
	"Numpad4",
	"Numpad5",
	"Numpad6",
	"Numpad7",
	"Numpad8",
	"Numpad9",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"F13",
	"F14",
	"F15",
	"Pause",
	"Tilde",
	"Dash",
	"BackSpace",
	"BackSlash",
	"SemiColon",
	"Return",
	"Quote"
};

string InputInterpreter::buttonName(int index, int input) {

	if(index == KEYBOARD_INDEX) {
		return keysToString[input];

	}else if(index == MOUSE_INDEX) {
		return "mb" + std::to_string(input);

	}else if(index >= 0 && index < sf::Joystick::Count) {
		
		if(input >= 0 && input < sf::Joystick::ButtonCount) {
			return "B" + std::to_string(input);

		}else {
			input -= sf::Joystick::ButtonCount;
			string axis = "Axis" + std::to_string(input / 2);			
			string dir = (input % 2 == 0) ? "+" : "-";

			return axis + dir;
		}
	}

	return "<Invalid Button>";
}