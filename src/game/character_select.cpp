#include "character_select.h"

#include "core/render_instance.h"
#include "core/input_interpreter.h"
#include "core/save.h"
#include "core/menu.h"

using std::vector, std::string;

struct Creator {

    enum ID {
        Save        = -1,
        Delete      = -2,
        Cancel      = -3,
        Disregard   = -4,
        Test        = -5,
        MoveList    = -6
    };

    enum Mode {
        List,
        Config,
        Move,
        Animation,
        Motion
    };

    Player dummy        = Player();
    bool test           = false;
    int confHover       = 0;
    int moveHover       = 0;
    int animHover       = 0;
    int confSelected    = 0;
    int moveSelected    = 0;
    int animSelected    = 0;
    string animSave     = "";
    int mode            = Mode::Config;

    vector<Menu::Option> getConfigOptions() {
        vector<Menu::Option> out;

        out.push_back({"MoveList", ID::MoveList});
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

        for(auto anim : g::save.getAnimationsByFilter(Move::toCategory(moveSelected))) 
            out.push_back({anim->name, ID::Disregard});

        out.push_back({"", ID::Disregard});

        out.push_back({"REMOVE", ID::Delete});
        out.push_back({"BACK", ID::Cancel});

        return out;
    }

    void update(int total) {

        // Dummy behavior
        Button::Flag in;

        if(test) {
            in = dummy.readInput();
            dummy.advanceFrame(in, {});

        }else {
            dummy.state.moveFrame ++;

            switch(mode) {

            case Creator::Mode::Config:
                dummy.setMove(Move::Stand, true);
                break;

            case Creator::Mode::Move:

                if(getMoveOptions()[moveHover].id >= 0)
                    dummy.setMove(getMoveOptions()[moveHover].id, true);  
                break;       

            case Creator::Mode::Animation:
                dummy.config.moves[moveSelected] = getAnimationOptions()[animHover].name;
                dummy.setMove(moveSelected, true);
                break;

            case Creator::Mode::Motion:
                dummy.setMove(moveSelected, true);
                break;                
            }
        }   

        // Reset state
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

        g::video.window.setView(view);
        dummy.draw();
        g::video.window.setView(g::video.window.getDefaultView());

        Rectangle area {
            (g::video.camera.screen_w / total * dummy.seatIndex) + 8, 
            g::video.camera.screen_h * 2.f / 3.f + 8,
            (g::video.camera.screen_w / total) - 16, 
            g::video.camera.screen_h * 1.f / 3.f - 16
        };

        if(test) {

            if(dummy.state.button[0].Taunt)
                test = false;

        }else if(mode == Mode::Config) {
            auto options = getConfigOptions();

            int res = Menu::List(options, &confHover, dummy.seatIndex, area);

            if(res == Menu::Accept) {

                // Toggle test
                if(options[confHover].id == ID::Test) {
                    test = true;

                }else if(options[confHover].id == ID::MoveList) {
                    moveHover = 0;
                    mode = Mode::Move;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::Config;
            }

        // Draw the movelist selection
        }else if(mode == Mode::Move) {
            auto options = getMoveOptions();

            int res = Menu::Table(options, 3, true, &moveHover, dummy.seatIndex, area);

            if(res == Menu::Accept) {

                if(options[moveHover].id == ID::Cancel) {
                    mode = Mode::Config;

                }else if(options[moveHover].id >= 0) {
                    moveSelected = options[moveHover].id;
                    animSave = dummy.config.moves[moveSelected];
                    mode = Mode::Animation;
                    animHover = 0;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::Config;
            }

        // Draw the animation selection
        }else if(mode == Mode::Animation) {
            auto options = getAnimationOptions();

            int res = Menu::List(options, &animHover, dummy.seatIndex, area);

            if(res == Menu::Accept) {

                if(options[animHover].id == ID::Delete) {
                    dummy.config.moves[moveSelected] = "";
                    dummy.config.motions[moveSelected] = "";
                    mode = Mode::Move;

                }else if(options[animHover].id == ID::Cancel) {
                    dummy.config.moves[moveSelected] = animSave;
                    mode = Mode::Move;

                }else {
                    dummy.config.moves[moveSelected] = options[animHover].name;
                    dummy.config.motions[moveSelected] = "";
                    mode = Mode::Motion;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::Move;
                dummy.config.moves[moveSelected] = animSave;
            }

        // Draw the motion interpreter
        }else if(mode == Mode::Motion) {

            if(Move::toCategory(moveSelected) == MoveCategory::Custom) {
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

    while (g::video.window.isOpen()) {
        g::input.prepEvents();

        sf::Event event;

        while(g::video.window.pollEvent(event)){
            g::input.processEvent(event);
        }

        if(g::input.windowClose)
            g::video.window.close();

        g::video.window.clear();

        for(int i = 0; i < creator.size(); i ++) 
            creator[i].update(creator.size());

        g::video.window.display();
    }

	return {};
}