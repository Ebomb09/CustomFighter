#include "lobby.h"
#include "local_game.h"
#include "net_game.h"
#include "practise_game.h"
#include "game_creation.h"
#include "character_select.h"
#include "options.h"

#include "core/menu.h"
#include "core/input_interpreter.h"
#include "core/video.h"
#include "core/save.h"
#include "core/scene_transition.h"

using std::vector, std::string;

namespace Screen {
    enum {
        Main,
        NetPlay,
        Versus,
        Practise,
        OptionMenu,
        Exit
    };
};

Vector2 pointsOfInterests[] {
    {275.f, -307.f},
    {817.f, -314.f},
    {588.f, -355.f},
    {1050.f, -615.f},
    {588.f, -355.f}
};

int main(int argc, char* argv[]) {

    g::video.setTitle("Custom Fighter");
    g::video.setSize(g::save.resolution);
    g::video.setDisplayMode(g::save.displayMode);
    g::video.setVSync(g::save.vsync);
    g::video.reload();

    int hover = 0;

    // Dynamic background
    sf::Texture* mainBG = g::save.getTexture("data/hud/main.png");
    int waitForInterest = -1;

    SceneTransition st;
    st.nextScene(SceneTransition::Open, Screen::Main);

    while (g::video.isOpen()) {
        g::input.pollEvents();
        g::video.clear();

        st.advanceFrame();

        // Draw the main bg
        if(mainBG) {

            Vector2 maxSize;

            if(g::video.getSize().x / g::video.getSize().y < (float)mainBG->getSize().x / (float)mainBG->getSize().y) {
                maxSize = {
                    (float)mainBG->getSize().y * g::video.getSize().x / g::video.getSize().y,
                    (float)mainBG->getSize().y
                };

            }else {
                maxSize = {
                    (float)mainBG->getSize().x,                    
                    (float)mainBG->getSize().x * g::video.getSize().y / g::video.getSize().x,
                };
            }

            if(waitForInterest != hover) 
                waitForInterest = -1;

            Vector2 desiredSize;
            Vector2 desiredPoint; 
            
            if(waitForInterest == -1) {
                desiredSize = {maxSize.x, maxSize.y};
                desiredPoint = {mainBG->getSize().x / 2.f, mainBG->getSize().y / -2.f};

            }else {
                desiredSize = {maxSize.x / 2.f, maxSize.y / 2.f};
                desiredPoint = pointsOfInterests[hover];
            }

            // Can pick a new interest
            if(waitForInterest == -1 && g::video.camera.w / desiredSize.x > 0.75f) 
                waitForInterest = hover;

            // Get center of camera
            Vector2 center = {g::video.camera.x + g::video.camera.w / 2.f, g::video.camera.y - g::video.camera.h / 2.f};

            // Adjust camera to the desired
            center.x += (desiredPoint.x - center.x) * 0.05f;
            center.y += (desiredPoint.y - center.y) * 0.05f;
            g::video.camera.w += (desiredSize.x - g::video.camera.w) * 0.05f;
            g::video.camera.h += (desiredSize.y - g::video.camera.h) * 0.05f;

            // Readjust camera to center
            g::video.camera.x = center.x - g::video.camera.w / 2.f;
            g::video.camera.y = center.y + g::video.camera.h / 2.f;

            g::video.camera.x = std::clamp(g::video.camera.x, 0.f, mainBG->getSize().x - g::video.camera.w);
            g::video.camera.y = std::clamp(g::video.camera.y, mainBG->getSize().y * -1.0f + g::video.camera.h, 0.f);
            g::video.camera.w = std::clamp(g::video.camera.w, 0.f, maxSize.x);
            g::video.camera.h = std::clamp(g::video.camera.h, 0.f, maxSize.y);

            sf::RectangleShape rect = g::video.toScreen({0.f, 0.f, (float)mainBG->getSize().x, (float)mainBG->getSize().y});
            rect.setTexture(mainBG);
            g::video.draw(rect);
        }

        // Create the menu
        Menu::Config mainMenu;
        
        mainMenu.draw_Area = {
            64,
            64,
            g::video.getSize().x / 2 - 128,
            g::video.getSize().y - 128
        };

        mainMenu.push_back({Screen::NetPlay, "NetPlay"});
        mainMenu.push_back({Screen::Versus, "Versus"});
        mainMenu.push_back({Screen::Practise, "Practise Mode"});
        mainMenu.push_back({Screen::OptionMenu, "Options"});
        mainMenu.push_back({Screen::Exit, "Exit"});

        int res = Menu::Table(mainMenu, 0, &hover, st.ready());

        // Draw fade in / out circle
        g::video.draw(st.getCircleFadeEffect({0.f, 0.f, g::video.getSize().x, g::video.getSize().y}));

        // Render before we run any other menus
        g::video.display();

        if(st.ready()) {

            // Check the screen state
            switch(st.scene()) {

            case Screen::Versus: {
                Lobby::Room room = GameCreation::run();
    
                if(room.code >= 0) {

                    // Next get the configs and insert them into the room
                    vector<Player::Config> playerConfigs = CharacterSelect::run(room.player_max);

                    for(int i = 0; i < playerConfigs.size(); i ++) {
                        room.player_data[i].config = playerConfigs[i];
                        room.player_count ++;
                    }

                    if(room.good())
                        LocalGame::run(room);
                } 

                st.nextScene(SceneTransition::Open, Screen::Main);
                break;
            }

            case Screen::Practise: {
                vector<Player::Config> configs = CharacterSelect::run(2);

                if(configs.size() == 2) 
                    PractiseGame::run(configs);

                st.nextScene(SceneTransition::Open, Screen::Main);
                break;
            }
                    
            case Screen::NetPlay: {
                vector<Player::Config> configs = CharacterSelect::run(1);

                if(configs.size() == 1) {
                    Lobby::Room room = Lobby::run(configs[0]);

                    if(room.good()) 
                        NetGame::run(room);
                }
                st.nextScene(SceneTransition::Open, Screen::Main);
                break;
            }

            case Screen::OptionMenu: {
                Options::run({mainMenu.draw_Area.x + 128, mainMenu.draw_Area.y, mainMenu.draw_Area.w, mainMenu.draw_Area.h});
                st.nextScene(SceneTransition::Open, Screen::Main);
                break;
            }

            case Screen::Exit: {
                g::video.close();
                break;
            }
            }

            if(res == Menu::Accept)
                st.nextScene(SceneTransition::Close, mainMenu[hover].id);
        }
    }

	return 0;
}