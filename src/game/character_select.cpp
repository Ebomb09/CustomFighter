#include "character_select.h"

#include "core/video.h"
#include "core/input_interpreter.h"
#include "core/save.h"
#include "core/audio.h"
#include "core/menu.h"
#include "core/scene_transition.h"

#include <SFML/Audio.hpp>
#include <chrono>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>

using std::vector, std::string;
using namespace std::chrono;

const vector<string> ConfigMoveCategories = {"Stance", "Custom Moves"};

struct Creator {

    enum ID {
        Disregard = -1,
        Save,
        Delete,
        Cancel,
        Test,
        MoveList,
        Confirm,
        Costume,
        Insert,
        ArmSize,
        LegSize,
        ShoulderSize,
        HipSize,
        Height,
        Index
    };

    enum Mode {
        ModifyConfig,
        ListConfigMoves,
        ListAnimations,
        SetConfigMotion,
        ListConfigItems,
        ListClothes,        
        SetConfigItemColor
    };

    Rectangle reference;
    Player dummy        = Player();
    bool test           = false;
    bool choosen        = false;
    bool done           = false;
    int total           = 0;
    int configHover     = 0;
    int modifyHover     = 0;
    int moveHover       = 0;
    int moveCategorySelected = 0;
    int animHover       = 0;
    int animCategorySelected = 0;
    int itemHover       = 0;
    int clothHover      = 0;
    int clothCategorySelected = 0;
    int configSelected  = 0;
    int moveSelected    = 0;
    int itemSelected    = 0;
    Player::Config      backup;
    int mode            = Mode::ModifyConfig;

    Menu::Config getModificationMenu() {
        Menu::Config conf;
        conf.draw_Area = getMenuDiv();

        conf.push_back({ID::Confirm, "Confirm"});
        conf.push_back({});
        conf.push_back({ID::Costume, "Costume"});
        conf.push_back({ID::MoveList, "MoveList"});
        conf.push_back({});
        conf.push_back({ID::Test, "Test"});
        conf.push_back({ID::Save, "Save"});
        conf.push_back({ID::Cancel, "Cancel"});

        return conf;
    }

    Menu::Config getConfigMenu() {
        Menu::Config conf;
        conf.draw_Area = getMenuDiv();

        conf.data_Header = "< " + ConfigMoveCategories[moveCategorySelected] + " (" + std::to_string(moveCategorySelected+1) + "/" + std::to_string(ConfigMoveCategories.size()) + ") >";
        conf.data_Columns = 3;
        conf.data_GroupByRow = true;

        for(int i = 0; i < Move::Total; i ++) {

            if((moveCategorySelected == 0 && i < Move::Custom00) ||
                (moveCategorySelected == 1 && i >= Move::Custom00)) {
                conf.push_back({ID::Index, i, Move::String[i]});
                conf.push_back({ID::Index, i, dummy.config.motions[i], "fight"});
                conf.push_back({ID::Index, i, dummy.config.moves[i]});   
            }    
        }
          
        // Spacing
        conf.push_back({});
        conf.push_back({});
        conf.push_back({});

        conf.push_back({ID::Cancel, "BACK"});
        conf.push_back({});
        conf.push_back({});

        return conf;
    }

    Menu::Config getAnimationMenu() {
        auto time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        Menu::Config conf;
        conf.draw_Area = getDivBelowBar();
        conf.draw_RowHeight = 128.f;

        vector<int> categories = Move::getValidCategories(moveSelected);
        string category = MoveCategory::String[categories[animCategorySelected]];
        
        conf.data_Header = "< " + category + " (" + std::to_string(animCategorySelected+1) + "/" + std::to_string(categories.size()) + ") >";
        conf.data_Columns = 2;
        conf.data_GroupByRow = true;

        // Add each animation in the category which isn't already equiped
        for(auto anim : g::save.getAnimationsByFilter({Move::getValidCategories(moveSelected)[animCategorySelected]})) {
            bool equiped = false;

            for(int i = 0; i < Move::Total; i ++) {

                if(dummy.config.moves[i] == anim) {
                    equiped = true;
                    break;
                }
            }

            if(!equiped) {
                conf.push_back({ID::Test, anim});

                Animation* ptr = g::save.getAnimation(anim);

                Player test = dummy;
                test.config.moves[moveSelected] = anim;
                test.state.moveIndex = moveSelected;

                if(ptr && ptr->getFrameCount() > 0) 
                    test.state.moveFrame = (time / 17) % ptr->getFrameCount();

                conf.push_back({0, test});
            }
        }

        conf.push_back({});
        conf.push_back({});

        // Moves before custom's are required
        if(moveSelected >= Move::Custom00) {
            conf.push_back({ID::Delete, "[-] Remove"});
            conf.push_back({});            
        }

        conf.push_back({ID::Cancel, "Back"});
        conf.push_back({});

        return conf;
    }

    Menu::Config getConfigCostumeMenu() {
        Menu::Config conf;
        conf.draw_Area = getMenuDiv();

        conf.data_Columns = 3;
        conf.data_GroupByRow = true;

        conf.push_back({ID::Disregard, "Proportions"});
        conf.push_back({});
        conf.push_back({});

        conf.push_back({ID::ArmSize, "4", "fight", 1});
        conf.push_back({ID::ArmSize, "Arm Size: " + std::to_string((int)(dummy.config.armSize * 12.5))});
        conf.push_back({ID::ArmSize, "6", "fight", -1});

        conf.push_back({ID::LegSize, "4", "fight", 1});
        conf.push_back({ID::LegSize, "Leg Size: " + std::to_string((int)(dummy.config.legSize * 11.76))});
        conf.push_back({ID::LegSize, "6", "fight", -1});

        conf.push_back({ID::ShoulderSize, "4", "fight", 1});
        conf.push_back({ID::ShoulderSize, "Shoulder Size: " + std::to_string((int)(dummy.config.shoulderSize * 100))});
        conf.push_back({ID::ShoulderSize, "6", "fight", -1});

        conf.push_back({ID::HipSize, "4", "fight", 1});
        conf.push_back({ID::HipSize, "Hip Size: " + std::to_string((int)(dummy.config.hipSize * 100))});
        conf.push_back({ID::HipSize, "6", "fight", -1});

        conf.push_back({ID::Height, "4", "fight", 1});
        conf.push_back({ID::Height, "Height: " + std::to_string((int)(dummy.config.height * 100))});
        conf.push_back({ID::Height, "6", "fight", -1});

        conf.push_back({});
        conf.push_back({});
        conf.push_back({});

        conf.push_back({ID::Disregard, "Clothes"});
        conf.push_back({});
        conf.push_back({});

        for(int i = 0; i < dummy.config.clothes.size(); i ++) {
            conf.push_back({ID::Index, i, std::to_string(i + 1) + ".", "Anton-Regular", 1});
            conf.push_back({ID::Index, i, dummy.config.clothes[i].name});

            if(i == 0)
                conf.push_back({ID::Index, i, "2 *", "fight", -1});
            else if(i == dummy.config.clothes.size()-1)
                conf.push_back({ID::Index, i, "@ 8", "fight", -1});
            else
                conf.push_back({ID::Index, i, "2 8", "fight", -1});
        }

        conf.push_back({ID::Insert, ""});
        conf.push_back({ID::Insert, "[+] Add"});
        conf.push_back({ID::Insert, ""});

        conf.push_back({});
        conf.push_back({});
        conf.push_back({});

        conf.push_back({ID::Cancel, "Back"});
        conf.push_back({ID::Cancel, ""});
        conf.push_back({ID::Cancel, ""});

        return conf;
    }

    Menu::Config getClothingMenu() {
        Menu::Config conf;
        
        conf.draw_Area = getMenuDiv();
        conf.data_Header = "< " + ClothingSpace::String[clothCategorySelected] + "(" + std::to_string(clothCategorySelected+1) + "/" + std::to_string(ClothingSpace::Total) + ") >";

        for(auto& clothing : g::save.getClothingList()) {
            bool equiped = false;

            // Check validity of the clothing
            Clothing* cloth = g::save.getClothing(clothing);

            if(!cloth)
                continue;

            // Check it has a part that is in the category we want
            if(!cloth->part[clothCategorySelected])
                continue;

            // Check already equiped
            for(auto& item : dummy.config.clothes) {

                if(item.name == clothing) {
                    equiped = true;
                    break;
                }
            }

            if(!equiped)
                conf.push_back({ID::Index, clothing});
        }
        
        conf.push_back({});
        conf.push_back({ID::Delete, "[-] Remove"});
        conf.push_back({ID::Cancel, "Back"});

        return conf;    
    }

    Rectangle getDiv() {
        return {
            reference.x + 16.f + reference.w / total * dummy.seatIndex,
            reference.y + 16.f,
            -32.f + reference.w / total,
            -32.f + reference.h
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

    Rectangle getHeaderBarDiv() {
        Rectangle div = getDiv();

        return {
            div.x,
            div.y,
            div.w,
            div.h * 1/16
        };
    }

    Rectangle getDivBelowBar() {
        Rectangle div = getDiv();
        Rectangle headerBar = getHeaderBarDiv();

        return {
            div.x,
            div.y + headerBar.h,
            div.w,
            div.h - headerBar.h
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
                txt.setFillColor(sf::Color(255, 255, 255));
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
            rect.setTexture(g::save.getTexture("data/hud/bar.png"));
            rect.setPosition({area.x + ((float)i / MAX_POINTS) * area.w, area.y});
            rect.setSize({area.w / MAX_POINTS, area.h});
            g::video.draw(rect);
        }

        if(current > MAX_POINTS)
            Menu::renderText("!TOO MANY MOVES!", "Anton-Regular", sf::Color(0,0,0), area, 0);
    }

    void update(Rectangle _reference, bool input) {
        reference = _reference;

        if(test) {

            // Capture player
            dummy.state.position.x = 0;
            dummy.in = dummy.readInput();
            vector<Player> others = {dummy};
            dummy.advanceFrame(others);

            Menu::renderPlayer(dummy, dummy.getRealBoundingBox(), getDiv());
            drawMoveProperties(getSubMenuDiv());

            // Return to editting
            if(input && dummy.state.button[0].Taunt) {
                test = false;
                done = false;
            }

        }else if(mode == Mode::ModifyConfig) {

            Menu::renderPlayer(dummy, dummy.getRealBoundingBox(), getPlayerDiv());
            vector<Player> others = {dummy};
            dummy.advanceFrame(others);
            drawPlayerPoints(getSubMenuDiv());
        
            backup = dummy.config;

            auto conf = getModificationMenu();
            int res = Menu::Table(conf, dummy.seatIndex, &modifyHover, input);

            if(res == Menu::Accept) {

                // Let user test their creation
                if(conf[modifyHover].id == ID::Test) {
                    test = true;

                // Save modifications
                }else if(conf[modifyHover].id == ID::Save) {

                    if(dummy.config.calculatePoints() <= MAX_POINTS) {
                        g::save.savePlayerConfig(configSelected, dummy.config);

                    }else {
                        g::audio.playSound(g::save.getSound("error"));
                    }

                // Confirm character
                }else if(conf[modifyHover].id == ID::Confirm) {

                    if(dummy.config.calculatePoints() <= MAX_POINTS) {
                        done = true;
                        test = true;
                        
                    }else {
                        g::audio.playSound(g::save.getSound("error"));
                    }

                // List config moves
                }else if(conf[modifyHover].id == ID::MoveList) {
                    mode = Mode::ListConfigMoves;

                // Return to previous config selection
                }else if(conf[modifyHover].id == ID::Cancel) {
                    choosen = false;

                // List worn items
                }else if(conf[modifyHover].id == ID::Costume) {
                    mode = Mode::ListConfigItems;
                }

            // Return to previous config selection
            }else if(res == Menu::Decline) {
                choosen = false;
            }

        // List the worn clothing items
        }else if(mode == Mode::ListConfigItems) {
            Menu::renderPlayer(dummy, dummy.getRealBoundingBox(), getPlayerDiv());
            vector<Player> others = {dummy};
            dummy.advanceFrame(others);

            auto conf = getConfigCostumeMenu();
            int res = Menu::Table(conf, dummy.seatIndex, &itemHover);

            if(res == Menu::Accept) {

                // Insert new clothing piece
                if(conf[itemHover].id == ID::Insert) {
                    backup = dummy.config;   

                    itemSelected = dummy.config.clothes.size();  
                    dummy.config.clothes.push_back({});
                    mode = Mode::ListClothes;

                }else if(conf[itemHover].id == ID::Cancel) {
                    mode = Mode::ModifyConfig;

                // Modify existing clothing piece
                }else if(conf[itemHover].id == ID::Index) {
                    backup = dummy.config;

                    itemSelected = conf[itemHover].data;
                    dummy.config.clothes[itemSelected].name = "";
                    mode = Mode::ListClothes;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::ModifyConfig;

            }else {
                Button::Config b = g::save.getButtonConfig(dummy.seatIndex);

                // Shift clothing order
                if(conf[itemHover].id == ID::Index) {
                    int index = conf[itemHover].data;

                    if(g::input.pressed(b.index, b.Left)) {
                        
                        if(index > 0) {
                            std::swap(dummy.config.clothes[index], dummy.config.clothes[index-1]);
                            itemHover -= 3;
                        }

                    }else if(g::input.pressed(b.index, b.Right)) {

                        if(index < dummy.config.clothes.size()-1) {
                            std::swap(dummy.config.clothes[index], dummy.config.clothes[index+1]);
                            itemHover += 3;
                        }
                    }

                // Shift proportion of arm size
                }else if(conf[itemHover].id == ID::ArmSize) {

                    if(g::input.held(b.index, b.Left)) 
                        dummy.config.armSize -= 0.1f;

                    else if(g::input.held(b.index, b.Right)) 
                        dummy.config.armSize += 0.1f;
                    
                    dummy.config.armSize = std::clamp(dummy.config.armSize, 3.f, 15.f);

                // Shift proportion of leg size
                }else if(conf[itemHover].id == ID::LegSize) {

                    if(g::input.held(b.index, b.Left))
                        dummy.config.legSize -= 0.1f;

                    else if(g::input.held(b.index, b.Right))
                        dummy.config.legSize += 0.1f;

                    dummy.config.legSize = std::clamp(dummy.config.legSize, 3.f, 15.f);  

                // Shift proportion of Shoulder size
                }else if(conf[itemHover].id == ID::ShoulderSize) {

                    if(g::input.held(b.index, b.Left))
                        dummy.config.shoulderSize -= 0.01f;

                    else if(g::input.held(b.index, b.Right))
                        dummy.config.shoulderSize += 0.01f;

                    dummy.config.shoulderSize = std::clamp(dummy.config.shoulderSize, 0.5f, 1.5f);

                // Shift proportion of Hip size
                }else if(conf[itemHover].id == ID::HipSize) {

                    if(g::input.held(b.index, b.Left))
                        dummy.config.hipSize -= 0.01f;

                    else if(g::input.held(b.index, b.Right))
                        dummy.config.hipSize += 0.01f;

                    dummy.config.hipSize = std::clamp(dummy.config.hipSize, 0.5f, 1.5f);

                // Shift proportion of Height
                }else if(conf[itemHover].id == ID::Height) {

                    if(g::input.held(b.index, b.Left))
                        dummy.config.height -= 0.01f;

                    else if(g::input.held(b.index, b.Right))
                        dummy.config.height += 0.01f;

                    dummy.config.height = std::clamp(dummy.config.height, 0.8f, 1.2f);          
                }
            }

        // List all available clothing items
        }else if(mode == Mode::ListClothes) {

            // Select clothing cateogry by parts in it
            Button::Config b = g::save.getButtonConfig(dummy.seatIndex);

            if(g::input.pressed(b.index, b.Left)) {
                clothCategorySelected --;
                clothHover = 0;
            }

            if(g::input.pressed(b.index, b.Right)) {
                clothCategorySelected ++;
                clothHover = 0;
            }

            if(clothCategorySelected < 0)
                clothCategorySelected = ClothingSpace::Total - 1;

            else if(clothCategorySelected >= ClothingSpace::Total)
                clothCategorySelected = 0;

            // List the selected clothing Category and Options
            auto options = getClothingMenu();
            int res = Menu::Table(options, dummy.seatIndex, &clothHover);

            vector<Player> others = {dummy};
            dummy.advanceFrame(others);
            
            // Highlight the clothing option hovered
            if(options[clothHover].id == ID::Index) {
                Player copy = dummy;
                auto time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count(); 

                copy.config.clothes[itemSelected].name = *options[clothHover].text;

                copy.config.clothes[itemSelected].r = 255;
                copy.config.clothes[itemSelected].g = 50 + std::sin(time * PI / 180 / 2) * 50;
                copy.config.clothes[itemSelected].b = 50 + std::sin(time * PI / 180 / 2) * 50;

                Menu::renderPlayer(copy, copy.getRealBoundingBox(), getPlayerDiv());

            }else {
                Menu::renderPlayer(dummy, dummy.getRealBoundingBox(), getPlayerDiv());
            } 

            if(res == Menu::Accept) {

                if(options[clothHover].id == ID::Delete) {
                    dummy.config.clothes.erase(dummy.config.clothes.begin() + itemSelected);
                    mode = Mode::ListConfigItems;  

                }else if(options[clothHover].id == ID::Cancel) {

                    // Rollback changes
                    dummy.config = backup;
                    mode = Mode::ListConfigItems;

                }else if(options[clothHover].id == ID::Index) {
                    dummy.config.clothes[itemSelected].name = *options[clothHover].text;

                    // Set default random colours
                    if(itemSelected >= backup.clothes.size()) {
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
            vector<Player> others = {dummy};
            dummy.advanceFrame(others);

            // Copy color in
            sf::Color color(dummy.config.clothes[itemSelected].r, dummy.config.clothes[itemSelected].g, dummy.config.clothes[itemSelected].b);

            Menu::Config conf;
            conf.draw_Area = getMenuDiv();

            int res = Menu::ColorPicker(conf, dummy.seatIndex, &color, input);

            // Copy color out
            dummy.config.clothes[itemSelected].r = color.r;
            dummy.config.clothes[itemSelected].g = color.g;
            dummy.config.clothes[itemSelected].b = color.b;

            if(res == Menu::Accept) {
                mode = ListConfigItems;

            }else if(res == Menu::Decline) {
                mode = Mode::ListClothes;
                dummy.config.clothes[itemSelected].name = "";
            }

        // Draw the movelist selection
        }else if(mode == Mode::ListConfigMoves) {
            backup = dummy.config;

            Menu::renderPlayer(dummy, dummy.getRealBoundingBox(), getPlayerDiv());
            drawMoveProperties(getSubMenuDiv());
            drawPlayerPoints(getSubMenuDiv());

            // Select to edit the player stance, or custom input moves
            Button::Config b = g::save.getButtonConfig(dummy.seatIndex);

            if(g::input.pressed(b.index, b.Left)) {
                moveCategorySelected --;
                moveHover = 0;
            }

            if(g::input.pressed(b.index, b.Right)) {
                moveCategorySelected ++;
                moveHover = 0;
            }

            if(moveCategorySelected < 0)
                moveCategorySelected = ConfigMoveCategories.size() - 1;

            else if(moveCategorySelected >= ConfigMoveCategories.size())
                moveCategorySelected = 0;

            auto conf = getConfigMenu();
            int res = Menu::Table(conf, dummy.seatIndex, &moveHover);

            // Set to default animation
            dummy.setMove(Move::Stand, true);
            vector<Player> others = {dummy};
            dummy.advanceFrame(others);

            if(res == Menu::Accept) {

                if(conf[moveHover].id == ID::Cancel) {
                    mode = Mode::ModifyConfig;

                }else if(conf[moveHover].id == ID::Index) {
                    backup = dummy.config;

                    moveSelected = conf[moveHover].data;
                    dummy.config.moves[moveSelected] = "";                    
                    dummy.config.motions[moveSelected] = "";
                    mode = Mode::ListAnimations;
                }

            }else if(res == Menu::Decline) {
                mode = Mode::ModifyConfig;

            }else {

                // Set the dummys animation
                if(conf[moveHover].id == ID::Index) {
                    Animation* anim = g::save.getAnimation(dummy.config.moves[conf[moveHover].data]);

                    if(anim) {
                        auto time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();  
                        dummy.state.moveIndex = conf[moveHover].data;                                         
                        dummy.state.moveFrame = (time / 17) % anim->getFrameCount();
                    }
                }               
            }

        // Draw the animation selection
        }else if(mode == Mode::ListAnimations) {

            // Choose category if multiple available
            Button::Config b = g::save.getButtonConfig(dummy.seatIndex);
            vector<int> categories = Move::getValidCategories(moveSelected);

            if(g::input.pressed(b.index, b.Left)) {
                animCategorySelected --;
                animHover = 0;
            }

            if(g::input.pressed(b.index, b.Right)) {
                animCategorySelected ++;
                animHover = 0;
            }

            if(animCategorySelected < 0)
                animCategorySelected = categories.size() - 1;

            else if(animCategorySelected >= categories.size())
                animCategorySelected = 0;

            auto conf = getAnimationMenu();
            int res = Menu::Table(conf, dummy.seatIndex, &animHover, input);

            // Show the points if selected
            if(conf[animHover].id == ID::Test)
                dummy.config.moves[moveSelected] = *conf[animHover].text;
            
            drawPlayerPoints(getHeaderBarDiv());

            // Revert to empty so the options continue working
            dummy.config.moves[moveSelected] = "";

            if(res == Menu::Accept) {

                // Delete animation and motions
                if(conf[animHover].id == ID::Delete) {
                    dummy.config.moves[moveSelected] = "";
                    dummy.config.motions[moveSelected] = "";
                    mode = Mode::ListConfigMoves;

                // Restore original, return to moveList
                }else if(conf[animHover].id == ID::Cancel) {
                    dummy.config = backup;
                    mode = Mode::ListConfigMoves;

                // Go to next step and select a motion
                }else if(conf[animHover].id == ID::Test){
                    dummy.config.moves[moveSelected] = *conf[animHover].text;
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

            drawPlayerPoints(getHeaderBarDiv());

            if(moveSelected >= Move::Custom00) {
                Menu::Config conf;
                conf.draw_Area = getDivBelowBar();

                int res = Menu::Motion(conf, dummy.seatIndex, &dummy.config.motions[moveSelected], input);

                if(res == Menu::Accept) 
                    mode = Mode::ListConfigMoves;

            }else {
                dummy.config.motions[moveSelected] = "";
                mode = Mode::ListConfigMoves;
            }
        }
   }
};

namespace Screen {
    enum {
        Main,
        CharacterSelect,
        CharacterCustomize,
        Game
    };
};

vector<Player::Config> CharacterSelect::run(int count) {

    // Invalid number of confs selected
    if(count <= 0)
        return {};

    sf::Sound* music = g::audio.playMusic(g::save.getMusic("Leaving Home"));
    music->setLoop(true);

    SceneTransition st;
    st.nextScene(SceneTransition::Open, Screen::CharacterSelect);

    vector<Creator> creator;

    for(int i = 0; i < count; i ++) {
        Creator cr;
        cr.total = count;
        cr.dummy.seatIndex = i;
        cr.configHover = i;
        creator.push_back(cr);
    }

    while (g::video.isOpen()) {
        g::input.pollEvents();
        g::video.clear();

        st.advanceFrame();

        // Scene div
        Rectangle div = st.getGrowthEffect(
            {16.f, 16.f, g::video.getSize().x - 32.f, g::video.getSize().y - 32.f},
            {g::video.getSize().x / 2.f, 16.f, 0.f, g::video.getSize().y - 32.f}
        );

        switch(st.scene()) {

            // Return to the main menu
            case Screen::Main: {
                return {};
            }

            // All done
            case Screen::Game: {
                vector<Player::Config> confs;

                for(int i = 0; i < creator.size(); i ++)
                    confs.push_back(creator[i].dummy.config);

                return confs;
            }

            // Update individual creators huds
            case Screen::CharacterCustomize: {
                int done = 0;

                for(int i = 0; i < creator.size(); i ++) {
                    creator[i].update(div, st.ready());
                
                    // Creator specified to return to character select
                    if(st.ready()) {

                        if(creator[i].done)
                            done ++;

                        if(!creator[i].choosen) {
                            st.nextScene(SceneTransition::CloseThenOpen, Screen::CharacterSelect);

                            // Still show this screen until transition back
                            for(int j = 0; j < creator.size(); j ++)
                                creator[j].choosen = false;
                        }
                    }
                }

                if(st.ready() && done == creator.size())
                    st.nextScene(SceneTransition::Close, Screen::Game);

                break;
            }

            // Update the unified character selector
            case Screen::CharacterSelect: {

                // Table options
                Menu::Config conf;
                conf.data_Columns = 5;

                conf.draw_Area = div;
                conf.draw_RowHeight = div.h / 6.f;

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

                    conf.push_back({i, test, capture});
                }

                Menu::Table(conf, 0, NULL, false);

                // Do the controls and state control
                int selected = 0;

                for(int i = 0; i < creator.size(); i ++) {

                    // Cursor position
                    int x = creator[i].configHover % 5;
                    int y = creator[i].configHover / 5;

                    // Determine player slot color
                    sf::Color color;
                    if(i == 0)
                        color = sf::Color(235, 67, 52);
                    else if(i == 1)
                        color = sf::Color(29, 62, 245);
                    else if(i == 2)
                        color = sf::Color(245, 220, 29);
                    else if(i == 3)
                        color = sf::Color(167, 28, 232);

                    // Expand the selection box
                    float expand = 0.f;

                    // In control
                    if(st.ready()) {

                        if(!creator[i].choosen) {
                            expand = 4.f + std::sin(st.counter() * PI / 180 * 5.f) * 4.f;

                            int res = Menu::DoControls(conf, creator[i].dummy.seatIndex, &creator[i].configHover, true);

                            // Selected a config player
                            if(res == Menu::Accept) {
                                creator[i].configSelected = conf[creator[i].configHover].id;
                                creator[i].dummy.config = g::save.getPlayerConfig(creator[i].configSelected);
                                creator[i].choosen = true;

                            // Exiting the character select
                            }else if(res == Menu::Decline) {
                                st.nextScene(SceneTransition::Close, Screen::Main);
                            }

                        }else{
                            selected ++;
                            Button::Config b = g::save.getButtonConfig(i);

                            // Change decision
                            if(g::input.pressed(b.index, b.D)) 
                                creator[i].choosen = false;
                        }
                    }

                    // Draw selection box
                    sf::RectangleShape sh;
                    sh.setPosition({div.x + x * div.w / 5.f + expand, div.y + y * div.h / 6.f + expand});
                    sh.setSize({div.w / 5.f - expand*2.f, div.h / 6.f - expand*2.f});
                    sh.setFillColor(color - sf::Color(0, 0, 0, 240));
                    sh.setOutlineColor(color);
                    sh.setOutlineThickness(-4.f);
                    g::video.draw(sh);

                    // Draw player identifier
                    sf::Text text;
                    text.setString("P" + std::to_string(i+1));
                    text.setFont(*g::save.getFont("Anton-Regular"));
                    text.setFillColor(color);

                    if(i == 0)
                        text.setPosition({div.x + x * div.w / 5.f + 12.f, div.y + y * div.h / 6.f + 12.f});
                    else if(i == 1)
                        text.setPosition({div.x + (x+1) * div.w / 5.f - 12.f - text.getGlobalBounds().width, div.y + y * div.h / 6.f + 12.f});
                    else if(i == 2)
                        text.setPosition({div.x + x * div.w / 5.f + 12.f, div.y + (y+1) * div.h / 6.f - 12.f - text.getGlobalBounds().height});
                    else if(i == 3)
                        text.setPosition({div.x + (x+1) * div.w / 5.f - 12.f - text.getGlobalBounds().width, div.y + (y+1) * div.h / 6.f - 12.f - text.getGlobalBounds().height});

                    g::video.draw(text);
                }

                // Not returning to main menu and all have selected
                if(st.ready() && selected == creator.size()) {
                    st.nextScene(SceneTransition::CloseThenOpen, Screen::CharacterCustomize);
                
                    // Get to start screen
                    for(int i = 0; i < creator.size(); i ++) {
                        creator[i].mode = Creator::ModifyConfig;
                        creator[i].test = false;
                        creator[i].done = false;
                    }
                }

                break;
            }
        }
        g::video.display();
    }

	return {};
}