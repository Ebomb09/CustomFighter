#include "character_select.h"

#include "core/render_instance.h"
#include "core/input_interpreter.h"
#include "core/save.h"
#include "core/menu.h"

#include <vector>
#include <stack>

using std::vector, std::string, std::stack;

struct Creator {

    enum ID {
        Save        = -1,
        Delete      = -2,
        Cancel      = -3,
        Disregard   = -4,
        Test        = -5,
        MoveList    = -6,
        Confirm     = -7,
        Costume     = -8,
        Insert      = -9
    };

    enum Mode {
        List,
        Config,
        Move,
        Animation,
        Motion,
        WornClothes,
        ColorClothes,
        ListClothes
    };

    Player dummy        = Player();
    bool test           = false;
    bool done           = false;
    bool exit           = false;
    int listHover       = 0;
    int confHover       = 0;
    int moveHover       = 0;
    int animHover       = 0;
    int wornHover       = 0;
    int clothHover      = 0;
    int clothColor      = 0;
    int listSelected    = 0;
    int moveSelected    = 0;
    int wornSelected    = 0;
    string moveSave     = "";
    Player::Config::Cloth wornSave;
    int mode            = Mode::List;

    vector<Menu::Option> getListOptions() {
        vector<Menu::Option> out;

        for(int i = 0; i < g::save.maxPlayerConfigs; i ++) {
            out.push_back({std::to_string(i), i});
        }
        return out;
    }

    vector<Menu::Option> getConfigOptions() {
        vector<Menu::Option> out;

        out.push_back({"Confirm", ID::Confirm});

        out.push_back({"", ID::Disregard});

        out.push_back({"Costume", ID::Costume});
        out.push_back({"MoveList", ID::MoveList});

        out.push_back({"", ID::Disregard});

        out.push_back({"Test", ID::Test});
        out.push_back({"Save", ID::Save});
        out.push_back({"Cancel", ID::Cancel});

        return out;
    }

    vector<Menu::Option> getMoveOptions() {
        vector<Menu::Option> out;

        for(int i = 0; i < Move::Total; i ++) {
            out.push_back({Move::String[i], i});
            out.push_back({dummy.config.motions[i], i, "fight"});
            out.push_back({dummy.config.moves[i], i});                        
        }
          
        // Spacing
        out.push_back({"", ID::Disregard});
        out.push_back({"", ID::Disregard});
        out.push_back({"", ID::Disregard});

        out.push_back({"BACK", ID::Cancel});
        out.push_back({"", ID::Disregard});
        out.push_back({"", ID::Disregard});

        return out;
    }

    vector<Menu::Option> getAnimationOptions() {
        vector<Menu::Option> out;

        for(auto anim : g::save.getAnimationsByFilter(Move::getValidCategories(moveSelected)))
            out.push_back({anim, ID::Test});

        out.push_back({"", ID::Disregard});

        out.push_back({"REMOVE", ID::Delete});
        out.push_back({"BACK", ID::Cancel});

        return out;
    }

    vector<Menu::Option> getConfigClothes() {
        vector<Menu::Option> out;    
        
        for(int i = 0; i < dummy.config.clothes.size(); i ++) {
            out.push_back({dummy.config.clothes[i].name, i});
        }

        out.push_back({"", ID::Disregard});
        out.push_back({"ADD", ID::Insert});
        out.push_back({"BACK", ID::Cancel});

        return out;
    }

    vector<Menu::Option> getClothingOptions() {
        vector<Menu::Option> out;
        
        for(auto& clothing : g::save.getClothingList())
            out.push_back({clothing, 0});
        
        out.push_back({"", ID::Disregard});
        out.push_back({"REMOVE", ID::Delete});
        out.push_back({"BACK", ID::Cancel});

        return out;    
    }

    void update(int total) {

        // Dummy behavior
        Button::Flag in;

        if(test) {
            dummy.in = dummy.readInput();
            dummy.advanceFrame({});

        }else {

            if(dummy.doneMove())
                dummy.setMove(Move::Stand, true);

            dummy.state.moveFrame ++;            
        }   

        // Reset state
        g::video.camera.w = CameraBounds.w;
        g::video.camera.h = CameraBounds.w * g::video.getSize().y / g::video.getSize().x;

        dummy.state.position.x = 0;

        // Get the position of the skeleton
        Rectangle box = dummy.getScreenBoundingBox();
        box.x -= 64;
        box.y -= 64;
        box.w += 128;
        box.h += 128;

        // Capture the player position
        sf::View view = sf::View(sf::FloatRect(box.x, box.y, box.w, box.h));

        // Draw to the view port and maintain aspect ratio
        float w = (1.f / 3.f) * (box.w / box.h);
        float h = (1.f / 3.f);
        float x = (1.f + dummy.seatIndex * 2.f) / (2.f * total) - w / 2.f;
        float y = 1.f / 3.f;

        view.setViewport(sf::FloatRect(x, y, w, h));

        g::video.setView(view);
        dummy.draw();
        g::video.setView(g::video.getDefaultView());

        Rectangle area {
            ((float)g::video.getSize().x / total * dummy.seatIndex) + 8, 
            (float)g::video.getSize().y * 2.f / 3.f + 8,
            ((float)g::video.getSize().x / total) - 16, 
            (float)g::video.getSize().y * 1.f / 3.f - 16
        };

        if(test) {

            if(dummy.state.button[0].Taunt) {
                test = false;
                done = false;
            }

        }else if(mode == Mode::List) {
            auto options = getListOptions();

            int res = Menu::Table(options, 5, false, &listHover, dummy.seatIndex, area);

            dummy.config = g::save.getPlayerConfig(options[listHover].id);

            if(res == Menu::Accept) {
                listSelected = options[listHover].id;
                mode = Mode::Config;

            // Exiting the character select
            }else if(res == Menu::Decline) {
                exit = true;
            }

        }else if(mode == Mode::Config) {
            auto options = getConfigOptions();

            int res = Menu::List(options, &confHover, dummy.seatIndex, area);

            if(res == Menu::Accept) {

                // Toggle test
                if(options[confHover].id == ID::Test) {
                    test = true;

                }else if(options[confHover].id == ID::Save) {
                    g::save.savePlayerConfig(listSelected, dummy.config);

                }else if(options[confHover].id == ID::Confirm) {
                    done = true;
                    test = true;

                }else if(options[confHover].id == ID::MoveList) {
                    mode = Mode::Move;

                }else if(options[confHover].id == ID::Cancel) {
                    mode = Mode::List;

                }else if(options[confHover].id == ID::Costume) {
                    mode = Mode::WornClothes;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::List;
            }

        }else if(mode == Mode::WornClothes) {
            auto options = getConfigClothes();

            int res = Menu::List(options, &wornHover, dummy.seatIndex, area);

            if(res == Menu::Accept) {

                if(options[wornHover].id == ID::Insert) {
                    dummy.config.clothes.push_back({""});
                    wornSelected = dummy.config.clothes.size()-1;
                    wornSave = Player::Config::Cloth();
                    mode = Mode::ListClothes;

                }else if(options[wornHover].id == ID::Cancel) {
                    mode = Mode::Config;

                }else {
                    wornSelected = options[wornHover].id;
                    wornSave = dummy.config.clothes[wornSelected];
                    mode = Mode::ListClothes;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::Config;
            }

        }else if(mode == Mode::ListClothes) {
            auto options = getClothingOptions();

            int res = Menu::List(options, &clothHover, dummy.seatIndex, area);

            if(options[clothHover].id >= 0)
                dummy.config.clothes[wornSelected].name = options[clothHover].name;

            if(res == Menu::Accept) {

                if(options[clothHover].id == ID::Delete) {
                    dummy.config.clothes.erase(dummy.config.clothes.begin() + wornSelected);
                    mode = Mode::WornClothes;  

                }else if(options[clothHover].id == ID::Cancel) {

                    // Rollback changes
                    if(wornSave.name.size() == 0)
                        dummy.config.clothes.erase(dummy.config.clothes.begin() + wornSelected);
                    else
                        dummy.config.clothes[wornSelected] = wornSave;

                    mode = Mode::WornClothes;

                }else {
                    mode = Mode::ColorClothes;
                }

            }else if(res == Menu::Decline) {

                // Rollback changes
                if(wornSave.name.size() == 0)
                    dummy.config.clothes.erase(dummy.config.clothes.begin() + wornSelected);
                else
                    dummy.config.clothes[wornSelected] = wornSave;

                mode = Mode::WornClothes;
            }

        }else if(mode == Mode::ColorClothes) {

            sf::Color color(dummy.config.clothes[wornSelected].r, dummy.config.clothes[wornSelected].g, dummy.config.clothes[wornSelected].b);

            int res = Menu::ColorPicker(&color, dummy.seatIndex, area);

            dummy.config.clothes[wornSelected].r = color.r;
            dummy.config.clothes[wornSelected].g = color.g;
            dummy.config.clothes[wornSelected].b = color.b;

            if(res == Menu::Accept) {
                dummy.config.clothes[wornSelected].name = getClothingOptions()[clothHover].name;
                mode = WornClothes;

            }else if(res == Menu::Decline) {
                mode = Mode::ListClothes;
            }

        // Draw the movelist selection
        }else if(mode == Mode::Move) {
            auto options = getMoveOptions();

            int res = Menu::Table(options, 3, true, &moveHover, dummy.seatIndex, area);

            if(options[moveHover].id >= 0)
                dummy.setMove(getMoveOptions()[moveHover].id, true); 

            if(res == Menu::Accept) {

                if(options[moveHover].id == ID::Cancel) {
                    mode = Mode::Config;

                }else if(options[moveHover].id >= 0) {
                    moveSelected = options[moveHover].id;
                    moveSave = dummy.config.moves[moveSelected];
                    mode = Mode::Animation;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::Config;
            }

        // Draw the animation selection
        }else if(mode == Mode::Animation) {
            auto options = getAnimationOptions();

            int res = Menu::List(options, &animHover, dummy.seatIndex, area);

            if(options[animHover].id == ID::Test) {
                dummy.config.moves[moveSelected] = options[animHover].name;
                dummy.setMove(getMoveOptions()[moveHover].id, true);
            }

            if(res == Menu::Accept) {

                if(options[animHover].id == ID::Delete) {
                    dummy.config.moves[moveSelected] = "";
                    dummy.config.motions[moveSelected] = "";
                    mode = Mode::Move;

                }else if(options[animHover].id == ID::Cancel) {
                    dummy.config.moves[moveSelected] = moveSave;
                    mode = Mode::Move;

                }else {
                    dummy.config.moves[moveSelected] = options[animHover].name;
                    dummy.config.motions[moveSelected] = "";
                    mode = Mode::Motion;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::Move;
                dummy.config.moves[moveSelected] = moveSave;
            }

        // Draw the motion interpreter
        }else if(mode == Mode::Motion) {

            if(moveSelected >= Move::Custom00) {
                int res = Menu::Motion(&dummy.config.motions[moveSelected], dummy.seatIndex, area);

                if(res == Menu::Accept) 
                    mode = Mode::Move;

            }else {
                dummy.config.motions[moveSelected] = "";
                mode = Mode::Move;
            }
        }
    }
};

vector<Player::Config> CharacterSelect::run(int count) {

    vector<Creator> creator;

    for(int i = 0; i < count; i ++) {
        Creator cr;
        cr.dummy.seatIndex = i;
        creator.push_back(cr);
    }

    while (g::video.isOpen()) {
        g::input.prepEvents();

        sf::Event event;

        while(g::video.pollEvent(event)){
            g::input.processEvent(event);
        }

        g::video.clear();

        vector<Player::Config> confs;

        for(int i = 0; i < creator.size(); i ++) {
            creator[i].update(creator.size());

            // Exit the menu
            if(creator[i].exit)
                return {};

            if(creator[i].done) 
                confs.push_back(creator[i].dummy.config);
        }

        // All confs filled in and done
        if(confs.size() == creator.size())
            return confs;

        g::video.display();
    }

	return {};
}