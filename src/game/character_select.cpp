#include "character_select.h"

#include "core/render_instance.h"
#include "core/input_interpreter.h"
#include "core/save.h"


bool CharacterSelect::run(Player::Config conf) {

	vector<int> selected;
	selected.resize(4, -1);

    while (g::video.window.isOpen()) {
        g::input.prepEvents();

        sf::Event event;

        while(g::video.window.pollEvent(event)){
            ImGui::SFML::ProcessEvent(event);

            if(!ImGui::GetIO().WantCaptureKeyboard && !ImGui::GetIO().WantCaptureMouse)
                g::input.processEvent(event);
        }

        if(g::input.windowClose)
            g::video.window.close();

        ImGui::SFML::Update(g::video.window, g::video.clock.restart());

        g::video.window.clear();

        ImGui::Begin("Character Select");

        // Show each move in the configuration
        for(int i = 0; i < Move::Total; i ++) {

        	// Dropdown each valid animation
        	if(ImGui::BeginCombo(Move::String[i].c_str(), conf.moves[i].c_str())) {

        		for(auto it : g::save.animations) {

        			// Only show if the animation is in the move category
        			if(it.second->category[i]) {

        				if(ImGui::Selectable(it.first.c_str())) 
        					conf.moves[i] = it.first;        				
        			}
        		}

        		ImGui::EndCombo();
        	}
        }

        ImGui::End();


        ImGui::SFML::Render(g::video.window);
        g::video.window.display();
    }

	return true;
}