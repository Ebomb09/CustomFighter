#include "game_creation.h"
#include "character_select.h"

#include "core/input_interpreter.h"
#include "core/video.h"
#include "core/menu.h"
#include "core/save.h"
#include "core/scene_transition.h"

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

static vector<vector<int>> presets {
    {GameMode::Versus, GameMode::Rounds},
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 30},
    {30, 60, 99},
    {2, 4}
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
		Menu::Config conf;
		conf.draw_Area = st.getGrowthEffect(
            {
                16.f,
                16.f,
                g::video.getSize().x - 32.f,
                g::video.getSize().y - 32.f,
            },
            {
                16.f,
                g::video.getSize().y / 2.f,
                g::video.getSize().x - 32.f,
                0.f,
            }
        );

        conf.data_Columns = 12;
        conf.data_GroupByRow = true;

        // Game Mode Selection
		conf.push_back({ID::GameMode, "Game Mode:"});
        for(int i = 0; i < conf.data_Columns-1; i ++)
            conf.push_back({});

        for(int i = 0; i < conf.data_Columns; i ++) {

            if(i < presets[ID::GameMode].size()) {

                if(selected[ID::GameMode] == i) {
                    conf.push_back({-1, ">" + GameMode::String[i] + "<"});
                }else{
                    conf.push_back({-1, GameMode::String[i]});
                }
            }else{
                conf.push_back({});
            }
        }

        // Round Max Selection
		conf.push_back({ID::RoundMax, "Round Count:"});
        for(int i = 0; i < conf.data_Columns-1; i ++)
            conf.push_back({});

        for(int i = 0; i < conf.data_Columns; i ++) {

            if(i < presets[ID::RoundMax].size()) {

                if(selected[ID::RoundMax] == i) {
                    conf.push_back({-1, ">" + std::to_string(presets[ID::RoundMax][i]) + "<"});
                }else{
                    conf.push_back({-1, std::to_string(presets[ID::RoundMax][i])});
                }
            }else{
                conf.push_back({});
            }
        }

        // Timer Max Selection
		conf.push_back({ID::TimerMax, "Round Time:"});
        for(int i = 0; i < conf.data_Columns-1; i ++)
            conf.push_back({});

        for(int i = 0; i < conf.data_Columns; i ++) {

            if(i < presets[ID::TimerMax].size()) {

                if(selected[ID::TimerMax] == i) {
                    conf.push_back({-1, ">" + std::to_string(presets[ID::TimerMax][i]) + "<"});
                }else{
                    conf.push_back({-1, std::to_string(presets[ID::TimerMax][i])});
                }
            }else{
                conf.push_back({});
            }
        }

        // Player Max Selection
		conf.push_back({ID::PlayerMax, "Players:"});
        for(int i = 0; i < conf.data_Columns-1; i ++)
            conf.push_back({});

        for(int i = 0; i < conf.data_Columns; i ++) {

            if(i < presets[ID::PlayerMax].size()) {

                if(selected[ID::PlayerMax] == i) {
                    conf.push_back({-1, ">" + std::to_string(presets[ID::PlayerMax][i]) + "<"});
                }else{
                    conf.push_back({-1, std::to_string(presets[ID::PlayerMax][i])});
                }
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
                    selected[index] ++;

                if(g::input.pressed(b.index, b.Left))
                    selected[index] --;

                if(selected[index] >= presets[index].size())
                    selected[index] = 0;

                if(selected[index] < 0)
                    selected[index] = presets[index].size()-1;
            }

            // Signalled done
            if(res == Menu::Accept && conf[hover].id == ID::Accept) {
                st.nextScene(SceneTransition::Close, Scene::Done);

            // Signalled back
            }else if((res == Menu::Accept && conf[hover].id == ID::Decline) || res == Menu::Decline) {
                st.nextScene(SceneTransition::Close, Scene::Back);
            }
        }
		g::video.display();
	}
    return Lobby::Room();
}