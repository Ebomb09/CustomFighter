#include "game_creation.h"
#include "character_select.h"

#include "core/input_interpreter.h"
#include "core/video.h"
#include "core/menu.h"
#include "core/save.h"
#include "core/scene_transition.h"

#include <iostream>
#include <cmath>
#include <vector>
#include <string>

using std::string, std::vector;

namespace Scene {
    enum {
        Back,
        Open,
        Done
    };
};

namespace ID {
    enum {
        GameMode,
        RoundMax,
        TimerMax,
        PlayerMax,
        Accept,
        Decline
    };
};

static int getMultiple(int x, int m) {
    return x + (m - x % m);
}

static vector<vector<int>> presets {
    {GameMode::Versus, GameMode::Rounds},
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 30},
    {30, 60, 99},
    {2, 4}
};

static vector<vector<Rectangle>> borders {
    vector<Rectangle>(presets[0].size()),
    vector<Rectangle>(presets[1].size()),
    vector<Rectangle>(presets[2].size()),
    vector<Rectangle>(presets[3].size())
};

Lobby::Room GameCreation::run() {
	int hover = 0;

    SceneTransition st;
    st.nextScene(SceneTransition::Open, Scene::Open);

    int selected[] {
        0,
        2,
        2,
        0
    };

	while (g::video.isOpen()) {
	    g::input.pollEvents();
	    g::video.clear();

        st.advanceFrame();

        Button::Config b = g::save.getButtonConfig(0);

		// Create the options menu
        float uniSize = std::min(g::video.getSize().x - 32.f, g::video.getSize().y - 32.f);

		Menu::Config conf;
		conf.draw_Area = st.getGrowthEffect(
            {
                g::video.getSize().x / 2.f - uniSize / 2.f,
                g::video.getSize().y / 2.f - uniSize / 2.f,
                uniSize,
                uniSize,
            },
            {
                g::video.getSize().x / 2.f,
                g::video.getSize().y / 2.f - uniSize / 2.f,
                0.f,
                uniSize,
            }
        );

        conf.data_Columns = 4;
        conf.draw_RowHeight = uniSize / 17.f;
        conf.data_GroupByRow = true;

        // Game Mode Selection
		conf.push_back({ID::GameMode, "Game Mode:"});
        for(int i = 0; i < conf.data_Columns-1; i ++)
            conf.push_back({});

        for(int i = 0; i < getMultiple(presets[ID::GameMode].size(), conf.data_Columns); i ++) {

            if(i < presets[ID::GameMode].size()) {

                if(selected[ID::GameMode] == i) {
                    conf.push_back({-1, GameMode::String[i]});
                    Menu::getCell(conf, conf.size()-1, 0, NULL, NULL, &borders[ID::GameMode][i]);
                }else{
                    conf.push_back({-1, GameMode::String[i]});
                }
            }else{
                conf.push_back({});
            }
        }

        for(int i = 0; i < conf.data_Columns; i ++)
            conf.push_back({});

        // Round Max Selection
		conf.push_back({ID::RoundMax, "Round Count:"});
        for(int i = 0; i < conf.data_Columns-1; i ++)
            conf.push_back({});

        for(int i = 0; i < getMultiple(presets[ID::RoundMax].size(), conf.data_Columns); i ++) {

            if(i < presets[ID::RoundMax].size()) {
                conf.push_back({-1, std::to_string(presets[ID::RoundMax][i])});
                Menu::getCell(conf, conf.size()-1, 0, NULL, NULL, &borders[ID::RoundMax][i]);
            }else{
                conf.push_back({});
            }
        }

        for(int i = 0; i < conf.data_Columns; i ++)
            conf.push_back({});

        // Timer Max Selection
		conf.push_back({ID::TimerMax, "Round Time:"});
        for(int i = 0; i < conf.data_Columns-1; i ++)
            conf.push_back({});

        for(int i = 0; i < getMultiple(presets[ID::TimerMax].size(), conf.data_Columns); i ++) {

            if(i < presets[ID::TimerMax].size()) {
                conf.push_back({-1, std::to_string(presets[ID::TimerMax][i])});
                Menu::getCell(conf, conf.size()-1, 0, NULL, NULL, &borders[ID::TimerMax][i]);
            }else{
                conf.push_back({});
            }
        }

        for(int i = 0; i < conf.data_Columns; i ++)
            conf.push_back({});

        // Player Max Selection
		conf.push_back({ID::PlayerMax, "Players:"});
        for(int i = 0; i < conf.data_Columns-1; i ++)
            conf.push_back({});

        for(int i = 0; i < getMultiple(presets[ID::PlayerMax].size(), conf.data_Columns); i ++) {

            if(i < presets[ID::PlayerMax].size()) {
                conf.push_back({-1, std::to_string(presets[ID::PlayerMax][i])});
                Menu::getCell(conf, conf.size()-1, 0, NULL, NULL, &borders[ID::PlayerMax][i]);
            }else{
                conf.push_back({});
            }
        }

        for(int i = 0; i < conf.data_Columns; i ++)
            conf.push_back({});

		conf.push_back({ID::Accept, "Start"});
        for(int i = 0; i < conf.data_Columns-1; i ++)
            conf.push_back({});

		conf.push_back({ID::Decline, "Back"});
        for(int i = 0; i < conf.data_Columns-1; i ++)
            conf.push_back({});

		int res = Menu::Table(conf, 0, &hover, true);

        if(st.ready()) {

            // Return an invalid Lobby Room
            if(st.scene() == Scene::Back) {
                return Lobby::Room();

            // Construct a valid Lobby Room, even for offline play
            }else if(st.scene() == Scene::Done) {
                Lobby::Room room;
                room.code = 0;
                room.game_mode = presets[ID::GameMode][selected[ID::GameMode]];
                room.round_max =  presets[ID::RoundMax][selected[ID::RoundMax]];
                room.timer_max = presets[ID::TimerMax][selected[ID::TimerMax]];
                room.player_max = presets[ID::PlayerMax][selected[ID::PlayerMax]];
                return room;
            }

            // Cycle the selected index
            if(conf[hover].id >= ID::GameMode && conf[hover].id <= ID::PlayerMax) {
                int index = conf[hover].id;

                if(g::input.pressed(b.index, b.Right))

                    if(selected[index] >= presets[index].size() - 1)
                        selected[index] = 0;
                    else
                        selected[index] ++;

                if(g::input.pressed(b.index, b.Left)) {

                    if(selected[index] == 0)
                        selected[index] = presets[index].size() - 1;
                    else
                        selected[index] --;
                }
            }

            // Signalled done
            if(res == Menu::Accept && conf[hover].id == ID::Accept) {
                st.nextScene(SceneTransition::Close, Scene::Done);

            // Signalled back
            }else if((res == Menu::Accept && conf[hover].id == ID::Decline) || res == Menu::Decline) {
                st.nextScene(SceneTransition::Close, Scene::Back);
            }

            // Draw the selected outlines
            for(int i = 0; i < presets.size(); i ++) {
                sf::RectangleShape sh = borders[i][selected[i]];
                sh.setFillColor(sf::Color::Transparent);
                sh.setOutlineColor(sf::Color::Red);
                sh.setOutlineThickness(-2.f);
                g::video.draw(sh);
            }
        }
		g::video.display();
	}
    return Lobby::Room();
}