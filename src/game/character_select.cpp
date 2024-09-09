#include "character_select.h"

#include "core/render_instance.h"
#include "core/input_interpreter.h"
#include "core/save.h"
#include "core/menu.h"

#include <vector>
#include <string>

using std::vector, std::string;

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
        ChooseConfig,
        ModifyConfig,
        ListConfigMoves,
        ListAnimations,
        SetConfigMotion,
        ListConfigItems,
        ListClothes,        
        SetConfigItemColor
    };

    Player dummy        = Player();
    bool test           = false;
    bool done           = false;
    bool exit           = false;
    int configHover     = 0;
    int modifyHover     = 0;
    int moveHover       = 0;
    int animHover       = 0;
    int itemHover       = 0;
    int clothHover      = 0;
    int configSelected  = 0;
    int moveSelected    = 0;
    int itemSelected    = 0;
    string moveSave     = "";
    Player::Config::Cloth itemSave;
    int mode            = Mode::ChooseConfig;

    vector<Menu::Option> getPlayerConfigOptions() {
        vector<Menu::Option> out;

        for(int i = 0; i < g::save.maxPlayerConfigs; i ++) {
            out.push_back({std::to_string(i), i});
        }
        return out;
    }

    vector<Menu::Option> getModificationOptions() {
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

    vector<Menu::Option> getConfigMoves() {
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

        Rectangle div {
            16.f + (float)g::video.getSize().x / total * dummy.seatIndex,
            16.f,
            -32.f + (float)g::video.getSize().x / total,
            -32.f + (float)g::video.getSize().y
        };

        // Capture player
        dummy.state.position.x = 0;

        Rectangle playerDiv = {
            div.x,
            div.y,
            div.w,
            div.h / 2
        };

        Rectangle boundingBox = dummy.getRealBoundingBox();
        boundingBox.x -= 16.f;
        boundingBox.y += 16.f;
        boundingBox.w += 32.f;
        boundingBox.h += 32.f;

        // Scale bounding box to proper aspect ratio
        Vector2 center = {boundingBox.x + boundingBox.w / 2, boundingBox.y - boundingBox.h / 2};

        if(boundingBox.w / boundingBox.h < playerDiv.w / playerDiv.h) {
            boundingBox.w = boundingBox.h * playerDiv.w / playerDiv.h;

        }else {
            boundingBox.h = boundingBox.w * playerDiv.h / playerDiv.w;
        }

        g::video.camera = {
            center.x - boundingBox.w / 2,
            center.y + boundingBox.h / 2,
            boundingBox.w,
            boundingBox.h
        };

        sf::View view = g::video.getDefaultView();
        view.setViewport(playerDiv.getScreenRatio());
        g::video.setView(view);

        dummy.draw();

        g::video.setView(g::video.getDefaultView());

        // Draw Menus / Features
        Rectangle menuDiv {
            div.x,
            div.y + div.h / 2,
            div.w,
            div.h / 2
        };

        if(test) {

            if(dummy.state.button[0].Taunt) {
                test = false;
                done = false;
            }

        }else if(mode == Mode::ChooseConfig) {
            auto options = getPlayerConfigOptions();

            int res = Menu::Table(options, 5, false, &configHover, dummy.seatIndex, menuDiv);

            dummy.config = g::save.getPlayerConfig(options[configHover].id);

            if(res == Menu::Accept) {
                configSelected = options[configHover].id;
                mode = Mode::ModifyConfig;

            // Exiting the character select
            }else if(res == Menu::Decline) {
                exit = true;
            }

        }else if(mode == Mode::ModifyConfig) {
            auto options = getModificationOptions();

            int res = Menu::List(options, &modifyHover, dummy.seatIndex, menuDiv);

            if(res == Menu::Accept) {

                // Toggle test
                if(options[modifyHover].id == ID::Test) {
                    test = true;

                }else if(options[modifyHover].id == ID::Save) {
                    g::save.savePlayerConfig(configSelected, dummy.config);

                }else if(options[modifyHover].id == ID::Confirm) {
                    done = true;
                    test = true;

                }else if(options[modifyHover].id == ID::MoveList) {
                    mode = Mode::ListConfigMoves;

                }else if(options[modifyHover].id == ID::Cancel) {
                    mode = Mode::ChooseConfig;

                }else if(options[modifyHover].id == ID::Costume) {
                    mode = Mode::ListConfigItems;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::ChooseConfig;
            }

        }else if(mode == Mode::ListConfigItems) {
            auto options = getConfigClothes();

            int res = Menu::List(options, &itemHover, dummy.seatIndex, menuDiv);

            if(res == Menu::Accept) {

                if(options[itemHover].id == ID::Insert) {
                    dummy.config.clothes.push_back({""});
                    itemSelected = dummy.config.clothes.size()-1;
                    itemSave = Player::Config::Cloth();
                    mode = Mode::ListClothes;

                }else if(options[itemHover].id == ID::Cancel) {
                    mode = Mode::ModifyConfig;

                }else {
                    itemSelected = options[itemHover].id;
                    itemSave = dummy.config.clothes[itemSelected];
                    mode = Mode::ListClothes;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::ModifyConfig;
            }

        }else if(mode == Mode::ListClothes) {
            auto options = getClothingOptions();

            int res = Menu::List(options, &clothHover, dummy.seatIndex, menuDiv);

            if(options[clothHover].id >= 0)
                dummy.config.clothes[itemSelected].name = options[clothHover].name;

            if(res == Menu::Accept) {

                if(options[clothHover].id == ID::Delete) {
                    dummy.config.clothes.erase(dummy.config.clothes.begin() + itemSelected);
                    mode = Mode::ListConfigItems;  

                }else if(options[clothHover].id == ID::Cancel) {

                    // Rollback changes
                    if(itemSave.name.size() == 0)
                        dummy.config.clothes.erase(dummy.config.clothes.begin() + itemSelected);
                    else
                        dummy.config.clothes[itemSelected] = itemSave;

                    mode = Mode::ListConfigItems;

                }else {
                    mode = Mode::SetConfigItemColor;
                }

            }else if(res == Menu::Decline) {

                // Rollback changes
                if(itemSave.name.size() == 0)
                    dummy.config.clothes.erase(dummy.config.clothes.begin() + itemSelected);
                else
                    dummy.config.clothes[itemSelected] = itemSave;

                mode = Mode::ListConfigItems;
            }

        }else if(mode == Mode::SetConfigItemColor) {

            sf::Color color(dummy.config.clothes[itemSelected].r, dummy.config.clothes[itemSelected].g, dummy.config.clothes[itemSelected].b);

            int res = Menu::ColorPicker(&color, dummy.seatIndex, menuDiv);

            dummy.config.clothes[itemSelected].r = color.r;
            dummy.config.clothes[itemSelected].g = color.g;
            dummy.config.clothes[itemSelected].b = color.b;

            if(res == Menu::Accept) {
                dummy.config.clothes[itemSelected].name = getClothingOptions()[clothHover].name;
                mode = ListConfigItems;

            }else if(res == Menu::Decline) {
                mode = Mode::ListClothes;
            }

        // Draw the movelist selection
        }else if(mode == Mode::ListConfigMoves) {
            auto options = getConfigMoves();

            int res = Menu::Table(options, 3, true, &moveHover, dummy.seatIndex, menuDiv);

            if(options[moveHover].id >= 0)
                dummy.setMove(getConfigMoves()[moveHover].id, true); 

            if(res == Menu::Accept) {

                if(options[moveHover].id == ID::Cancel) {
                    mode = Mode::ModifyConfig;

                }else if(options[moveHover].id >= 0) {
                    moveSelected = options[moveHover].id;
                    moveSave = dummy.config.moves[moveSelected];
                    mode = Mode::ListAnimations;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::ModifyConfig;
            }

        // Draw the animation selection
        }else if(mode == Mode::ListAnimations) {
            auto options = getAnimationOptions();

            int res = Menu::List(options, &animHover, dummy.seatIndex, menuDiv);

            if(options[animHover].id == ID::Test) {
                dummy.config.moves[moveSelected] = options[animHover].name;
                dummy.setMove(getConfigMoves()[moveHover].id, true);
            }

            if(res == Menu::Accept) {

                if(options[animHover].id == ID::Delete) {
                    dummy.config.moves[moveSelected] = "";
                    dummy.config.motions[moveSelected] = "";
                    mode = Mode::ListConfigMoves;

                }else if(options[animHover].id == ID::Cancel) {
                    dummy.config.moves[moveSelected] = moveSave;
                    mode = Mode::ListConfigMoves;

                }else {
                    dummy.config.moves[moveSelected] = options[animHover].name;
                    dummy.config.motions[moveSelected] = "";
                    mode = Mode::SetConfigMotion;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::ListConfigMoves;
                dummy.config.moves[moveSelected] = moveSave;
            }

        // Draw the motion interpreter
        }else if(mode == Mode::SetConfigMotion) {

            if(moveSelected >= Move::Custom00) {
                int res = Menu::Motion(&dummy.config.motions[moveSelected], dummy.seatIndex, menuDiv);

                if(res == Menu::Accept) 
                    mode = Mode::ListConfigMoves;

            }else {
                dummy.config.motions[moveSelected] = "";
                mode = Mode::ListConfigMoves;
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