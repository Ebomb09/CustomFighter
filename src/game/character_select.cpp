#include "character_select.h"

#include "core/render_instance.h"
#include "core/input_interpreter.h"
#include "core/save.h"
#include "core/menu.h"

#include <chrono>
#include <cmath>
#include <vector>
#include <string>

using std::vector, std::string;
using namespace std::chrono;

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
    int total           = 0;
    int configHover     = 0;
    int modifyHover     = 0;
    int moveHover       = 0;
    int moveCategorySelected = 0;
    int animHover       = 0;
    int animCategorySelected = 0;
    int itemHover       = 0;
    int clothHover      = 0;
    int configSelected  = 0;
    int moveSelected    = 0;
    int itemSelected    = 0;
    Player::Config      backup;
    int mode            = Mode::ChooseConfig;

    vector<Menu::Option> getPlayerConfigOptions() {
        vector<Menu::Option> out;

        for(int i = 0; i < g::save.maxPlayerConfigs; i ++) {

            Player test;
            test.config = g::save.getPlayerConfig(i);

            Skeleton pose = test.getSkeleton();

            Rectangle capture {
                pose.head.x - 0.5f,
                pose.head.y - 0.5f,
                1,
                1
            };

            out.push_back({i, test, capture});
        }
        return out;
    }

    vector<Menu::Option> getModificationOptions() {
        vector<Menu::Option> out;

        out.push_back({ID::Confirm, "Confirm"});

        out.push_back({});

        out.push_back({ID::Costume, "Costume"});
        out.push_back({ID::MoveList, "MoveList"});

        out.push_back({});

        out.push_back({ID::Test, "Test"});
        out.push_back({ID::Save, "Save"});
        out.push_back({ID::Cancel, "Cancel"});

        return out;
    }

    vector<Menu::Option> getConfigMoves() {
        vector<Menu::Option> out;

        for(int i = 0; i < Move::Total; i ++) {

            if((moveCategorySelected == 0 && i < Move::Custom00) ||
                (moveCategorySelected == 1 && i >= Move::Custom00)) {
                out.push_back({i, Move::String[i]});
                out.push_back({i, dummy.config.motions[i], "fight"});
                out.push_back({i, dummy.config.moves[i]});                  
            }    
        }
          
        // Spacing
        out.push_back({});
        out.push_back({});
        out.push_back({});

        out.push_back({ID::Cancel, "BACK"});
        out.push_back({});
        out.push_back({});

        return out;
    }

    vector<Menu::Option> getAnimationOptions() {
        auto time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        vector<Menu::Option> out;

        for(auto anim : g::save.getAnimationsByFilter({Move::getValidCategories(moveSelected)[animCategorySelected]})) {
            out.push_back({ID::Test, anim});

            Animation* ptr = g::save.getAnimation(anim);

            Player test = dummy;
            test.config.moves[moveSelected] = anim;
            test.state.moveIndex = moveSelected;

            if(ptr && ptr->getFrameCount() > 0) 
                test.state.moveFrame = (time / 17) % ptr->getFrameCount();

            out.push_back({0, test});
        }

        out.push_back({});
        out.push_back({});

        // Moves before custom's are required
        if(moveSelected >= Move::Custom00) {
            out.push_back({ID::Delete, "REMOVE"});
            out.push_back({});            
        }

        out.push_back({ID::Cancel, "BACK"});
        out.push_back({});

        return out;
    }

    vector<Menu::Option> getConfigClothes() {
        vector<Menu::Option> out;    
        
        for(int i = 0; i < dummy.config.clothes.size(); i ++) 
            out.push_back({i, dummy.config.clothes[i].name});
        
        out.push_back({});
        out.push_back({ID::Insert, "ADD"});
        out.push_back({ID::Cancel, "BACK"});

        return out;
    }

    vector<Menu::Option> getClothingOptions() {
        vector<Menu::Option> out;
        
        for(auto& clothing : g::save.getClothingList())
            out.push_back({0, clothing});
        
        out.push_back({});
        out.push_back({ID::Delete, "REMOVE"});
        out.push_back({ID::Cancel, "BACK"});

        return out;    
    }

    Rectangle getDiv() {
        return {
            16.f + (float)g::video.getSize().x / total * dummy.seatIndex,
            16.f,
            -32.f + (float)g::video.getSize().x / total,
            -32.f + (float)g::video.getSize().y
        };
    }

    Rectangle getMenuDiv() {
        Rectangle div = getDiv();

        return {
            div.x,
            div.y + div.h / 2,
            div.w,
            div.h / 2
        };
    }

    Rectangle getSubMenuDiv() {
        Rectangle div = getDiv();

        return {
            div.x,
            div.y,
            div.w / 2,
            div.h / 2
        };
    }

    Rectangle getPlayerDiv() {
        Rectangle div = getDiv();

        return {
            div.x,
            div.y,
            div.w,
            div.h / 2
        };
    }

    void drawMoveProperties(Rectangle area) {
        area.x += 16;
        area.w -= 32;
        area.y += 16;
        area.h -= 32;

        // Draw move properties
        float pos = 0;

        Animation* anim = g::save.getAnimation(dummy.config.moves[dummy.state.moveIndex]);

        if(anim) {
            int startup = anim->getStartup();
            int damage = anim->getDamage();
            int onhit = anim->getOnHit();
            int onblock = anim->getOnBlock();

            float width = 0;
            float fontHeight = 16;

            sf::Text txt;
            txt.setFont(*g::save.getFont("Anton-Regular"));                
            txt.setCharacterSize(fontHeight);
            txt.setOutlineColor(sf::Color(0, 0, 0));
            txt.setOutlineThickness(2);

            // Draw properties
            txt.setString("Category: ");
            txt.setPosition({area.x, area.y + pos});
            g::video.draw(txt);
            pos += fontHeight;
            width = std::max(width, txt.getLocalBounds().width);

            if(startup != -1) {
                txt.setString("Startup: ");
                txt.setPosition({area.x, area.y + pos});           
                g::video.draw(txt);
                pos += fontHeight;
                width = std::max(width, txt.getLocalBounds().width);

                txt.setString("On Hit: ");
                txt.setPosition({area.x, area.y + pos});
                g::video.draw(txt);
                pos += fontHeight;
                width = std::max(width, txt.getLocalBounds().width);

                txt.setString("On Block: ");
                txt.setPosition({area.x, area.y + pos});
                g::video.draw(txt);
                pos += fontHeight;
                width = std::max(width, txt.getLocalBounds().width);

                txt.setString("Damage: ");
                txt.setPosition({area.x, area.y + pos});
                g::video.draw(txt);
                pos += fontHeight;
                width = std::max(width, txt.getLocalBounds().width);                    
            }    

            bool customOnly = (anim->customFrom.size() > 0);

            if(customOnly) {

                for(int i = 0; i < MoveCategory::Total; i ++){

                    if(anim->from[i]){
                        customOnly = false;
                        break;
                    }
                }
            }

            if(customOnly) {
                pos += 32;
                txt.setString("Requires: ");
                txt.setPosition({area.x, area.y + pos});
                g::video.draw(txt);
                pos += fontHeight;
                width = std::max(width, txt.getLocalBounds().width);                    
            }

            // Draw property values
            pos = 0;
            txt.setString(MoveCategory::String[anim->category]); 
            txt.setPosition({area.x + width, area.y + pos});
            g::video.draw(txt);
            pos += fontHeight;    

            if(startup != -1) {
                txt.setString(std::to_string(startup));
                txt.setPosition({area.x + width, area.y + pos});
                txt.setFillColor(sf::Color(255, 255, 255));
                g::video.draw(txt);
                pos += fontHeight;

                txt.setString((onhit >= 0) ? "+" + std::to_string(onhit) : std::to_string(onhit));
                txt.setPosition({area.x + width, area.y + pos});
                txt.setFillColor((onhit >= 0) ? sf::Color(152, 150, 255) : sf::Color(255, 162, 156));
                g::video.draw(txt);
                pos += fontHeight;

                txt.setString((onblock >= 0) ? "+" + std::to_string(onblock) : std::to_string(onblock));
                txt.setPosition({area.x + width, area.y + pos});
                txt.setFillColor((onblock >= 0) ? sf::Color(152, 150, 255) : sf::Color(255, 162, 156));
                g::video.draw(txt);
                pos += fontHeight;

                txt.setString(std::to_string(damage));
                txt.setPosition({area.x + width, area.y + pos});
                txt.setFillColor(sf::Color(255, 0, 0));
                g::video.draw(txt);
                pos += fontHeight;               
            }

            if(customOnly) {
                pos += 32;
                txt.setString(anim->customFrom);
                txt.setColor(sf::Color(255, 255, 255));
                txt.setPosition({area.x + width, area.y + pos});
                g::video.draw(txt);
                pos += fontHeight;                 
            }
        }
        
    }

    void drawPlayerPoints(Rectangle area) {
        area.x += 16;
        area.w -= 32;

        area.y += area.h - 16 - 32;
        area.h = 32;

        int old = backup.calculatePoints();
        int current = dummy.config.calculatePoints();

        auto time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        for(int i = 0; i < MAX_POINTS; i ++) {
            sf::Color color(26, 26, 26);

            if(current > MAX_POINTS) {
                color = sf::Color(150 + std::sin(time * PI / 180 / 4) * 100, 0, 0);

            }else if(current > old) {
                
                if(i < old)
                    color = sf::Color(255, 255, 255);

                else if(i < current)
                    color = sf::Color(0, 150 + std::sin(time * PI / 180 / 4) * 100, 0);
            }else {

                if(i < current)
                    color = sf::Color(255, 255, 255);

                else if(i < old)
                    color = sf::Color(150 + std::sin(time * PI / 180 / 4) * 100, 0, 0);
            }

            sf::RectangleShape rect;
            rect.setFillColor(color);
            rect.setOutlineColor(sf::Color::Black);
            rect.setOutlineThickness(2);
            rect.setPosition({area.x + ((float)i / MAX_POINTS) * area.w, area.y});
            rect.setSize({area.w / MAX_POINTS, area.h});
            g::video.draw(rect);
        }

        if(current > MAX_POINTS)
            Menu::renderText("!TOO MANY MOVES!", "Anton-Regular", sf::Color(0,0,0), area, 0);
    }

    void update() {

        if(test) {

            // Capture player
            dummy.state.position.x = 0;            
            dummy.in = dummy.readInput();
            dummy.advanceFrame({});

            Menu::renderPlayer(dummy, dummy.getRealBoundingBox(), getDiv());
            drawMoveProperties(getSubMenuDiv());

            if(dummy.state.button[0].Taunt) {
                test = false;
                done = false;
            }

        }else if(mode == Mode::ChooseConfig) {

            auto options = getPlayerConfigOptions();
            int res = Menu::Table(options, 5, false, &configHover, dummy.seatIndex, getDiv(), 64);

            dummy.config = g::save.getPlayerConfig(options[configHover].id);

            if(res == Menu::Accept) {
                configSelected = options[configHover].id;
                mode = Mode::ModifyConfig;

            // Exiting the character select
            }else if(res == Menu::Decline) {
                exit = true;
            }

        }else if(mode == Mode::ModifyConfig) {

            Menu::renderPlayer(dummy, dummy.getRealBoundingBox(), getPlayerDiv());
            dummy.advanceFrame({});
            drawPlayerPoints(getSubMenuDiv());
        
            backup = dummy.config;

            auto options = getModificationOptions();
            int res = Menu::List(options, &modifyHover, dummy.seatIndex, getMenuDiv());

            if(res == Menu::Accept) {

                // Let user test their creation
                if(options[modifyHover].id == ID::Test) {
                    test = true;

                // Save modifications
                }else if(options[modifyHover].id == ID::Save) {

                    if(dummy.config.calculatePoints() <= MAX_POINTS) {
                        g::save.savePlayerConfig(configSelected, dummy.config);
                    }

                // Confirm character
                }else if(options[modifyHover].id == ID::Confirm) {

                    if(dummy.config.calculatePoints() <= MAX_POINTS) {
                        done = true;
                        test = true;
                    }

                // List config moves
                }else if(options[modifyHover].id == ID::MoveList) {
                    mode = Mode::ListConfigMoves;

                // Return to previous config selection
                }else if(options[modifyHover].id == ID::Cancel) {
                    mode = Mode::ChooseConfig;

                // List worn items
                }else if(options[modifyHover].id == ID::Costume) {
                    mode = Mode::ListConfigItems;
                }

            // Return to previous config selection
            }else if(res == Menu::Decline) {
                mode = Mode::ChooseConfig;
            }

        // List the worn clothing items
        }else if(mode == Mode::ListConfigItems) {
            Menu::renderPlayer(dummy, dummy.getRealBoundingBox(), getPlayerDiv());
            dummy.advanceFrame({});

            auto options = getConfigClothes();
            int res = Menu::List(options, &itemHover, dummy.seatIndex, getMenuDiv());

            if(res == Menu::Accept) {

                if(options[itemHover].id == ID::Insert) {
                    backup = dummy.config;   
                                     
                    dummy.config.clothes.push_back({});
                    itemSelected = dummy.config.clothes.size()-1;
                    mode = Mode::ListClothes;

                }else if(options[itemHover].id == ID::Cancel) {
                    mode = Mode::ModifyConfig;

                }else {
                    itemSelected = options[itemHover].id;
                    backup = dummy.config;
                    mode = Mode::ListClothes;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::ModifyConfig;
            }

        // List all available clothing items
        }else if(mode == Mode::ListClothes) {
            Menu::renderPlayer(dummy, dummy.getRealBoundingBox(), getPlayerDiv());
            dummy.advanceFrame({});

            auto options = getClothingOptions();
            int res = Menu::List(options, &clothHover, dummy.seatIndex, getMenuDiv());

            if(options[clothHover].id >= 0) {
                auto time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count(); 

                dummy.config.clothes[itemSelected].name = *options[clothHover].text;

                dummy.config.clothes[itemSelected].r = 255;
                dummy.config.clothes[itemSelected].g = 50 + std::sin(time * PI / 180 / 2) * 50;
                dummy.config.clothes[itemSelected].b = 50 + std::sin(time * PI / 180 / 2) * 50;
            }

            if(res == Menu::Accept) {

                if(options[clothHover].id == ID::Delete) {
                    dummy.config.clothes.erase(dummy.config.clothes.begin() + itemSelected);
                    mode = Mode::ListConfigItems;  

                }else if(options[clothHover].id == ID::Cancel) {

                    // Rollback changes
                    dummy.config = backup;
                    mode = Mode::ListConfigItems;

                }else {

                    // Reuse the backup colour if already specified
                    if(itemSelected < backup.clothes.size()) {
                        dummy.config.clothes[itemSelected].r = backup.clothes[itemSelected].r;
                        dummy.config.clothes[itemSelected].g = backup.clothes[itemSelected].g;
                        dummy.config.clothes[itemSelected].b = backup.clothes[itemSelected].b;

                    // Set default random colours
                    }else {
                        dummy.config.clothes[itemSelected].r = rand() % 256;
                        dummy.config.clothes[itemSelected].g = rand() % 256;
                        dummy.config.clothes[itemSelected].b = rand() % 256;                        
                    }
                    mode = Mode::SetConfigItemColor;
                }

            }else if(res == Menu::Decline) {

                // Rollback changes
                dummy.config = backup;
                mode = Mode::ListConfigItems;
            }

        }else if(mode == Mode::SetConfigItemColor) {
            Menu::renderPlayer(dummy, dummy.getRealBoundingBox(), getPlayerDiv());
            dummy.advanceFrame({});

            // Copy color in
            sf::Color color(dummy.config.clothes[itemSelected].r, dummy.config.clothes[itemSelected].g, dummy.config.clothes[itemSelected].b);

            int res = Menu::ColorPicker(&color, dummy.seatIndex, getMenuDiv());

            // Copy color out
            dummy.config.clothes[itemSelected].r = color.r;
            dummy.config.clothes[itemSelected].g = color.g;
            dummy.config.clothes[itemSelected].b = color.b;

            if(res == Menu::Accept) {
                mode = ListConfigItems;

            }else if(res == Menu::Decline) {
                mode = Mode::ListClothes;
            }

        // Draw the movelist selection
        }else if(mode == Mode::ListConfigMoves) {
            backup = dummy.config;

            Menu::renderPlayer(dummy, dummy.getRealBoundingBox(), getPlayerDiv());
            drawMoveProperties(getSubMenuDiv());
            drawPlayerPoints(getSubMenuDiv());

            // Select to edit the player stance, or custom input moves
            Button::Config b = g::save.getButtonConfig(dummy.seatIndex);
            vector<string> categories = {"Stance", "Custom Moves"};

            if(g::input.pressed(b.index, b.Left))
                moveCategorySelected --;

            if(g::input.pressed(b.index, b.Right))
                moveCategorySelected ++;

            if(moveCategorySelected < 0)
                moveCategorySelected = categories.size() - 1;

            else if(moveCategorySelected >= categories.size())
                moveCategorySelected = 0;

            Rectangle div = getMenuDiv();

            Rectangle headerDiv {
                div.x,
                div.y,
                div.w,
                div.h * 1/16
            };

            Rectangle menuDiv {
                div.x,
                headerDiv.y + headerDiv.h,
                div.w,
                div.h - headerDiv.h
            };

            // Draw which category we selected
            string header = "< " + categories[moveCategorySelected] + " >";
            Menu::renderText(header, "Anton-Regular", sf::Color::White, headerDiv, 0);

            auto options = getConfigMoves();
            int res = Menu::Table(options, 3, true, &moveHover, dummy.seatIndex, menuDiv);

            // Set to default animation
            dummy.setMove(Move::Stand, true);
            dummy.advanceFrame({});

            // Set the dummys animation
            if(options[moveHover].id >= 0) {
                Animation* anim = g::save.getAnimation(dummy.config.moves[options[moveHover].id]);

                if(anim) {
                    auto time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();  
                    dummy.state.moveIndex = options[moveHover].id;                                         
                    dummy.state.moveFrame = (time / 17) % anim->getFrameCount();
                }
            }

            if(res == Menu::Accept) {

                if(options[moveHover].id == ID::Cancel) {
                    mode = Mode::ModifyConfig;

                }else if(options[moveHover].id >= 0) {
                    backup = dummy.config;

                    moveSelected = options[moveHover].id;
                    dummy.config.moves[moveSelected] = "";                    
                    dummy.config.motions[moveSelected] = "";
                    mode = Mode::ListAnimations;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::ModifyConfig;
            }

        // Draw the animation selection
        }else if(mode == Mode::ListAnimations) {

            // Choose category if multiple available
            Button::Config b = g::save.getButtonConfig(dummy.seatIndex);
            vector<int> categories = Move::getValidCategories(moveSelected);

            if(g::input.pressed(b.index, b.Left))
                animCategorySelected --;

            if(g::input.pressed(b.index, b.Right))
                animCategorySelected ++;

            if(animCategorySelected < 0)
                animCategorySelected = categories.size() - 1;

            else if(animCategorySelected >= categories.size())
                animCategorySelected = 0;

            // List the availabile animations
            Rectangle div = getDiv();

            Rectangle barDiv {
                div.x,
                div.y,
                div.w,
                div.h * 1/16
            };   

            Rectangle headerDiv {
                div.x,
                barDiv.y + barDiv.h,
                div.w,
                div.h * 1/16
            };         

            Rectangle menuDiv {
                div.x,
                headerDiv.y + headerDiv.h,
                div.w,
                div.h - (barDiv.h + headerDiv.h)             
            };

            string category = MoveCategory::String[categories[animCategorySelected]];
            Menu::renderText("< " + category + " >", "Anton-Regular", sf::Color::White, headerDiv, 0);
            drawPlayerPoints(barDiv);

            auto options = getAnimationOptions();
            int res = Menu::Table(options, 2, true, &animHover, dummy.seatIndex, menuDiv, 128);

            // Set the dummy animation
            if(options[animHover].id == ID::Test)
                dummy.config.moves[moveSelected] = *options[animHover].text;
            else
                dummy.config.moves[moveSelected] = "";

            if(res == Menu::Accept) {

                // Delete animation and motions
                if(options[animHover].id == ID::Delete) {
                    dummy.config.moves[moveSelected] = "";
                    dummy.config.motions[moveSelected] = "";
                    mode = Mode::ListConfigMoves;

                // Restore original, return to moveList
                }else if(options[animHover].id == ID::Cancel) {
                    dummy.config = backup;
                    mode = Mode::ListConfigMoves;

                // Go to next step and select a motion
                }else if(options[animHover].id == ID::Test){
                    dummy.config.motions[moveSelected] = "";
                    mode = Mode::SetConfigMotion;
                }

            // Restore original, return to moveList
            }else if(res == Menu::Decline) {
                dummy.config = backup;
                mode = Mode::ListConfigMoves;
            }

        // Draw the motion interpreter
        }else if(mode == Mode::SetConfigMotion) {

            // List the availabile animations
            Rectangle div = getDiv();

            Rectangle barDiv {
                div.x,
                div.y,
                div.w,
                div.h * 1/16
            };           

            Rectangle menuDiv {
                div.x,
                div.y + div.h * 2/16,
                div.w,
                div.h - (div.h * 2/16)             
            };

            drawPlayerPoints(barDiv);

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
        cr.total = count;
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
            creator[i].update();

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