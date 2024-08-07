#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

#include "core/input_interpreter.h"
#include "core/render_instance.h"
#include "core/save.h"
#include "core/math.h"
#include "editor.h"

void Editor::drawGrid() {

    if(!settings.drawGrid)
        return;

    sf::Vertex l[2];
    float sX = std::floor(g::video.camera.x / 32) * 32;
    float sY = std::floor(g::video.camera.y / 32) * 32;

    for(int x = 0; x <= g::video.camera.w; x += 32) {
        l[0].position = g::video.camera.getScreen(Vector2{sX + x, g::video.camera.y});
        l[0].color = {20, 20, 20};
        l[1].position = g::video.camera.getScreen(Vector2{sX + x, g::video.camera.y - g::video.camera.h});
        l[1].color = {20, 20, 20};
        g::video.window.draw(l, 2, sf::PrimitiveType::Lines);
    }

    for(int y = 0; y <= g::video.camera.h; y += 32) {
        l[0].position = g::video.camera.getScreen(Vector2{g::video.camera.x, sY - y});
        l[0].color = {20, 20, 20};
        l[1].position = g::video.camera.getScreen(Vector2{g::video.camera.x + g::video.camera.w, sY - y});
        l[1].color = {20, 20, 20};
        g::video.window.draw(l, 2, sf::PrimitiveType::Lines);
    }

    // X - Line
    l[0].position = g::video.camera.getScreen(Vector2{g::video.camera.x, 0}); 
    l[0].color = sf::Color::Green;
    l[1].position = g::video.camera.getScreen(Vector2{g::video.camera.x + g::video.camera.w, 0});    
    l[1].color = sf::Color::Green;    
    g::video.window.draw(l, 2, sf::PrimitiveType::Lines);   

    // Y - Line
    l[0].position = g::video.camera.getScreen(Vector2{0, g::video.camera.y}); 
    l[0].color = sf::Color::Red;
    l[1].position = g::video.camera.getScreen(Vector2{0, g::video.camera.y - g::video.camera.h});
    l[1].color = sf::Color::Red;
    g::video.window.draw(l, 2, sf::PrimitiveType::Lines);
}

void Editor::drawModel() {

	if(!settings.drawModel)
		return;

    Skeleton copy;

    if(settings.playback)
        copy = player.getSkeleton();
    else
        copy = getSkeleton();

    for(int i = 0; i < copy.jointCount; i ++)
        copy.joints[i] = g::video.camera.getScreen(copy.joints[i]);

    std::vector<Clothing*> clothes = {
        g::save.getClothing("realistic")
    };

    copy.draw(clothes);	
}

void Editor::drawSkeleton() {

    if(!settings.drawSkeleton)
        return;

    if(keyFrame < 0)
        return;

    Skeleton copy;

    if(settings.playback)
        copy = player.getSkeleton();
    else
        copy = getSkeleton();

    for(int i = 0; i < copy.boneCount; i ++) {
        Bone& b = copy.bones[i];

        sf::Vertex line[2];
        line[0].position = g::video.camera.getScreen(b.start);
        line[0].color = {255, 255, 255, 255};
        line[1].position = g::video.camera.getScreen(b.end);
        line[1].color = {255, 255, 255, 255};

        g::video.window.draw((const sf::Vertex*)&line, 2, sf::PrimitiveType::Lines);
    }
}

void Editor::drawHitBox() {

    if(!settings.drawHitBox)
        return;

    if(keyFrame < 0)
        return;

    vector<HitBox> copy;

    if(settings.playback)
        copy = player.getHitBoxes();
    else
        copy = getHitBoxes();

    for(auto& box : copy) {
        sf::RectangleShape rect = g::video.camera.getScreen(box);
        rect.setFillColor({252, 62, 45, 50});
        rect.setOutlineThickness(1);
        rect.setOutlineColor({252, 62, 45});
        g::video.window.draw(rect);
    }  
}

void Editor::drawHurtBox() {
  
    if(!settings.drawHurtBox)
        return;

    if(keyFrame < 0)
        return;

    vector<HurtBox> copy;

    if(settings.playback)
        copy = player.getHurtBoxes();
    else
        copy = getHurtBoxes();

    for(auto& box : copy) {
        sf::RectangleShape rect = g::video.camera.getScreen(box);
        rect.setFillColor({252, 218, 45, 50});
        rect.setOutlineThickness(1);
        rect.setOutlineColor({252, 218, 45});
        g::video.window.draw(rect);
    }                
} 

void Editor::resetPlayer() {
    timer = 0;
    player.state.position = {0, 0};

    // Set player to test and address in the save
    g::save.animations["test"] = &anim;
    player.config.moves[Move::Custom00] = "test";
    player.setMove(Move::Custom00);
}

Skeleton Editor::getSkeleton() {

	if(keyFrame >= 0)
		return getKeyFrame().pose;

	return Skeleton();
}

std::vector<Rectangle*> Editor::getBoxes() {
    std::vector<Rectangle*> boxes;

    if(keyFrame < 0) {
        return boxes;

    }else if(settings.mode == Mode::HitBoxes) {

        for(int i = 0; i < getHitBoxes().size(); i ++)
            boxes.push_back(&getHitBoxes()[i]);

    }else if(settings.mode == Mode::HurtBoxes) {

        for(int i = 0; i < getHurtBoxes().size(); i ++)
            boxes.push_back(&getHurtBoxes()[i]);
    }
    return boxes;
}

Frame& Editor::getKeyFrame() {
    return anim.getKeyFrame(keyFrame);
}

std::vector<HitBox>& Editor::getHitBoxes() {
    return getKeyFrame().hitBoxes;
}

std::vector<HurtBox>& Editor::getHurtBoxes() {
    return getKeyFrame().hurtBoxes;
}  

void Editor::update() {
    drawGrid();
    drawModel();
    drawSkeleton();
    drawHitBox();
    drawHurtBox();

    // Editting modes
    if(keyFrame >= 0) {

        switch(settings.mode) {

        case Mode::Joints:
            selectJoint();
            break;

        case Mode::HitBoxes:
        case Mode::HurtBoxes:
            selectRectangle();
            break;
        }        
    }

    // Player simulation
    timer ++;

    if(settings.playback && timer >= settings.playbackSpeed) {
        timer = 0;

        player.advanceFrame(Button::Flag(), {player});

        if(player.inMove(Move::Stand))
            resetPlayer();
    }
}

void Editor::setKeyFrame(int index) {
    keyFrame = index;
    selected = -1;
    dragZone = -1;
}

void Editor::setSelected(int index) {
    selected = index;
    dragZone = -1;
}

void Editor::setDragZone(int mode) {
    dragZone = mode;
}

void Editor::selectDefault() {
    selected = -1;
    dragZone = -1;
}

void Editor::selectJoint() {
    Skeleton& skele = getKeyFrame().pose;     

    if(selected >= 0) {
        Vector2 j = g::video.camera.getScreen(skele.getJoint(selected));

        if(g::input.mousePressed[sf::Mouse::Button::Left]) {
            setDragZone(-1);

            if(Screen::pointInCircle(g::input.mousePosition, {j.x, j.y, 8}))
                setDragZone(1);
        }
    }

    if(g::input.mousePressed[sf::Mouse::Button::Left] && dragZone == -1) {
        bool pick_same = false;

        if(selected >= 0) {
            Vector2 j = g::video.camera.getScreen(skele.getJoint(selected));
            pick_same = Screen::pointInCircle(g::input.mousePosition, {j.x, j.y, 8});
        }
        
        if(!pick_same) {
            setSelected(-1);

            for(int i = 0; i < skele.jointCount; i ++) {
                Vector2 j = g::video.camera.getScreen(skele.joints[i]);

                if(Screen::pointInCircle(g::input.mousePosition, {j.x, j.y, 8})) {
                    setSelected(i);
                    break;
                }
            }
        }
    }

    if(g::input.mouseHeld[sf::Mouse::Button::Left] && selected >= 0 && dragZone == 1) {
        skele.moveJoint(selected,
            {
                g::input.mouseMove.x * g::video.camera.getScreenScale().x,
                -g::input.mouseMove.y * g::video.camera.getScreenScale().y
            }
        );
    }

    if(selected >= 0) {
        Vector2& joint = skele.getJoint(selected);

        Vector2 pos = g::video.camera.getScreen(joint);

        sf::CircleShape highlight; 
        highlight.setFillColor({0, 0, 0, 0});
        highlight.setOutlineColor({255, 0, 0});
        highlight.setOutlineThickness(2);
        highlight.setPosition({pos.x - 8, pos.y - 8});
        highlight.setRadius(8);
        g::video.window.draw(highlight);
    }     
}

void Editor::selectRectangle() {

    // Create Rectangles
    if(g::input.mousePressed[sf::Mouse::Button::Right]) {
        Vector2 pos = g::video.camera.getReal(g::input.mousePosition);
        
        if(settings.mode == Mode::HitBoxes) {
            getHitBoxes().push_back({pos.x - 25, pos.y + 25, 50, 50});
            setSelected(getHitBoxes().size() - 1);

        }else if(settings.mode == Mode::HurtBoxes) {
            getHurtBoxes().push_back({pos.x - 25, pos.y + 25, 50, 50});
            setSelected(getHurtBoxes().size() - 1);
        }
    }

    // Drag Zones
    if(selected >= 0) {
        std::vector<Rectangle*> boxes = getBoxes();

        Rectangle zone[9];

        for(int i = 0; i < 9; i ++) {

            if(i == 4)
                continue;

            Vector2 pos {
                boxes[selected]->x + (boxes[selected]->w/2) * (i % 3),
                boxes[selected]->y - (boxes[selected]->h/2) * (i / 3)
            };

            pos = g::video.camera.getScreen(pos);

            zone[i].x = pos.x - 4;
            zone[i].y = pos.y - 4;
            zone[i].w = 8;
            zone[i].h = 8;

            sf::RectangleShape rect = zone[i];
            rect.setFillColor(sf::Color::Transparent);
            rect.setOutlineColor(sf::Color::White);
            rect.setOutlineThickness(1);

            g::video.window.draw(rect);
        }

        if(g::input.mousePressed[sf::Mouse::Button::Left]) {
            setDragZone(-1);

            if(Screen::pointInRectangle(g::input.mousePosition, g::video.camera.getScreen(*boxes[selected])))
                setDragZone(4);

            for(int i = 0; i < 9; i ++) {

                if(i == 4)
                    continue;

                if(Screen::pointInRectangle(g::input.mousePosition, zone[i]))
                    setDragZone(i);
            }
        }
    }

    // Mouse in rectangle area
    if(g::input.mousePressed[sf::Mouse::Button::Left] && dragZone == -1) {
        std::vector<Rectangle*> boxes = getBoxes();        
        bool pick_same = false;

        if(selected >= 0) 
            pick_same = Screen::pointInRectangle(g::input.mousePosition, g::video.camera.getScreen(*boxes[selected]));
        
        if(!pick_same) {
            setSelected(-1);

            for(int i = 0; i < boxes.size(); i ++) {

                if(Screen::pointInRectangle(g::input.mousePosition, g::video.camera.getScreen(*boxes[i]))) {
                    setSelected(i);
                    break;
                }
            }                        
        }
    }

    if(g::input.mouseHeld[sf::Mouse::Button::Left] && selected >= 0) {
        std::vector<Rectangle*> boxes = getBoxes();
        Vector2 mov = g::input.mouseMove * g::video.camera.getScreenScale();

        switch(dragZone) {

        // Generic drag position
        case 4:
            boxes[selected]->x += mov.x;
            boxes[selected]->y += -mov.y;
            break;

        case 0:
            boxes[selected]->x += mov.x;
            boxes[selected]->y += -mov.y;
            boxes[selected]->w += -mov.x;
            boxes[selected]->h += -mov.y;
            break;

        case 1:
            boxes[selected]->y += -mov.y;              
            boxes[selected]->h += -mov.y;
            break;    

        case 2:
            boxes[selected]->y += -mov.y;            
            boxes[selected]->w += mov.x;
            boxes[selected]->h += -mov.y;
            break;   

        case 3:
            boxes[selected]->x += mov.x;
            boxes[selected]->w += -mov.x;
            break;

        case 5:
            boxes[selected]->w += mov.x;
            break;

        case 6:
            boxes[selected]->x += mov.x;
            boxes[selected]->w += -mov.x;
            boxes[selected]->h += mov.y;
            break;

        case 7:
            boxes[selected]->h += mov.y;
            break;

        case 8:
            boxes[selected]->w += mov.x;
            boxes[selected]->h += mov.y;
            break;      
        }
    }   

    if(g::input.keyPressed[sf::Keyboard::Delete] && selected >= 0) {

        if(settings.mode == Mode::HitBoxes) 
            getHitBoxes().erase(getHitBoxes().begin() + selected);

        else if(settings.mode == Mode::HurtBoxes) 
            getHurtBoxes().erase(getHurtBoxes().begin() + selected);
        
        setSelected(-1);
    }  
}