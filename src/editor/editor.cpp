#include <SFML/Graphics.hpp>
#include <cmath>

#include "core/input_interpreter.h"
#include "core/video.h"
#include "core/save.h"
#include "core/math.h"
#include "editor.h"

using std::vector, std::string;

void Editor::drawGrid() {

    if(!settings.drawGrid)
        return;

    sf::Vertex l[2];
    float sX = std::floor(g::video.camera.x / 32) * 32;
    float sY = std::floor(g::video.camera.y / 32) * 32;

    for(int x = 0; x <= g::video.camera.w; x += 32) {
        l[0].position = g::video.toScreen(Vector2{sX + x, g::video.camera.y});
        l[0].color = {20, 20, 20};
        l[1].position = g::video.toScreen(Vector2{sX + x, g::video.camera.y - g::video.camera.h});
        l[1].color = {20, 20, 20};
        g::video.draw(l, 2, sf::PrimitiveType::Lines);
    }

    for(int y = 0; y <= g::video.camera.h; y += 32) {
        l[0].position = g::video.toScreen(Vector2{g::video.camera.x, sY - y});
        l[0].color = {20, 20, 20};
        l[1].position = g::video.toScreen(Vector2{g::video.camera.x + g::video.camera.w, sY - y});
        l[1].color = {20, 20, 20};
        g::video.draw(l, 2, sf::PrimitiveType::Lines);
    }

    // X - Line
    l[0].position = g::video.toScreen(Vector2{g::video.camera.x, 0}); 
    l[0].color = sf::Color::Green;
    l[1].position = g::video.toScreen(Vector2{g::video.camera.x + g::video.camera.w, 0});    
    l[1].color = sf::Color::Green;    
    g::video.draw(l, 2, sf::PrimitiveType::Lines);   

    // Y - Line
    l[0].position = g::video.toScreen(Vector2{0, g::video.camera.y}); 
    l[0].color = sf::Color::Red;
    l[1].position = g::video.toScreen(Vector2{0, g::video.camera.y - g::video.camera.h});
    l[1].color = sf::Color::Red;
    g::video.draw(l, 2, sf::PrimitiveType::Lines);
}

void Editor::drawModel() {

	if(!settings.drawModel)
		return;

    vector<Skeleton> copy;

    if(settings.playback) {
        copy.push_back(player.getFrame().pose);

        if(player.getFrame().isGrab)
            copy.push_back(player.getFrame().grabee);

    }else {

        if(getPose()) 
            copy.push_back(*getPose());

        if(getGrabee())
            copy.push_back(*getGrabee());
    }

    for(int i = 0; i < copy.size(); i ++)
        copy[i].draw(player.getClothes());
}

void Editor::drawSkeleton() {

    if(!settings.drawSkeleton)
        return;

    if(keyFrame < 0)
        return;

    vector<Skeleton> copy;

    if(settings.playback) {
        copy.push_back(player.getSkeleton());

        if(player.getFrame().isGrab)
            copy.push_back(player.getFrame().grabee);

    }else {

        if(getPose()) 
            copy.push_back(*getPose());

        if(getGrabee())
            copy.push_back(*getGrabee());
    }

    for(int i = 0; i < copy.size(); i ++) {

        for(int j = 0; j < copy[i].boneCount; j ++) {
            Bone& b = copy[i].bones[j];

            sf::Vertex line[2];
            line[0].position = g::video.toScreen(b.start);
            line[0].color = {255, 255, 255, 255};
            line[1].position = g::video.toScreen(b.end);
            line[1].color = {255, 255, 255, 255};

            g::video.draw((const sf::Vertex*)&line, 2, sf::PrimitiveType::Lines);
        }
    }
}

void Editor::drawHitBox() {

    if(!settings.drawHitBox)
        return;

    if(keyFrame < 0)
        return;

    vector<HitBox> copy;

    if(settings.playback) {
        copy = player.getHitBoxes();
    }else {

        if(getHitBoxes())
            copy = *getHitBoxes();
    }

    for(auto& box : copy) {
        sf::RectangleShape rect = g::video.toScreen(box);
        rect.setFillColor({252, 62, 45, 50});
        rect.setOutlineThickness(1);
        rect.setOutlineColor({252, 62, 45});
        g::video.draw(rect);
    }  
}

void Editor::drawHurtBox() {
  
    if(!settings.drawHurtBox)
        return;

    if(keyFrame < 0)
        return;

    vector<HurtBox> copy;

    if(settings.playback) {
        copy = player.getHurtBoxes();
    }else {

        if(getHurtBoxes())
            copy = *getHurtBoxes();
    }

    for(auto& box : copy) {
        sf::RectangleShape rect = g::video.toScreen(box);
        rect.setFillColor({252, 218, 45, 50});
        rect.setOutlineThickness(1);
        rect.setOutlineColor({252, 218, 45});
        g::video.draw(rect);
    }                
} 

void Editor::resetPlayer() {
    timer = 0;
    player.state.position = {0, 0};
    player.state.velocity = {0, 0};
    player.state.moveFrame = 0;

    // Set player to test and copy in the save
    player.config.moves[Move::Stand] = "";
    player.config.moves[Move::Custom00] = "";

    string test = g::save.getAnimationsList()[0];
    *g::save.getAnimation(test) = anim;

    if(anim.category < Move::Custom00) {
        player.config.moves[Move::Stand] = test;
        player.state.moveIndex = Move::Stand;
        
    }else {
        player.config.moves[Move::Custom00] = test;
        player.state.moveIndex = Move::Custom00;     
    }
}

Frame* Editor::getKeyFrame(int key) {
    
    if(key == -1)
        key = keyFrame;

    if(key < 0 || key >= anim.keyFrames.size())
        return NULL;
    
    return &anim.keyFrames[key];
}

Skeleton* Editor::getPose(int key) {
    Frame* frame = getKeyFrame(key);

    if(frame)
        return &frame->pose;

	return NULL;
}

Skeleton* Editor::getGrabee(int key) {
    Frame* frame = getKeyFrame(key);

    if(frame && frame->isGrab)
        return &frame->grabee;

	return NULL;
}

vector<Vector2*> Editor::getJoints(int key) {
    Frame* frame = getKeyFrame(key);
    vector<Vector2*> joints;

    if(frame) {

        for(int i = 0; i < frame->pose.jointCount; i ++) 
            joints.push_back(&frame->pose.joints[i]);

        if(frame->isGrab) {
            for(int i = 0; i < frame->grabee.jointCount; i ++) 
                joints.push_back(&frame->grabee.joints[i]);
        }
    }
    return joints;
}

Vector2* Editor::getJoint(int index, int key) {
    vector<Vector2*> joints = getJoints(key);

    if(settings.mode == Mode::Joints && index == -1)
        index = selected;

    if(index < 0 || index >= joints.size())
        return NULL;

    return joints[index];
}

Skeleton* Editor::getJointOwner(int index, int key) {
    Vector2* joint = getJoint(index, key);
    Skeleton* pose = getPose();
    Skeleton* grabee = getGrabee();

    // Check if the memory location matches for either of the skeletons
    if(pose) {

        for(int i = 0; i < pose->jointCount; i ++)
            if(&pose->joints[i] == joint)
                return pose;
    }

    if(grabee) {

        for(int i = 0; i < grabee->jointCount; i ++)
            if(&grabee->joints[i] == joint)
                return grabee;
    }
    return NULL;
}

vector<Rectangle*> Editor::getBoxes(int key) {
    Frame* frame = getKeyFrame(key);
    vector<Rectangle*> boxes;

    if(frame) {

        if(settings.mode == Mode::HitBoxes) {

            for(auto& box : frame->hitBoxes)
                boxes.push_back(&box);

        }else if(settings.mode == Mode::HurtBoxes) {

            for(auto& box : frame->hurtBoxes)
                boxes.push_back(&box);
        }
    }
    return boxes;
}

Rectangle* Editor::getBox(int index, int key) {
    vector<Rectangle*> boxes = getBoxes(key);

    if(index == -1 && (settings.mode == Mode::HitBoxes || settings.mode == Mode::HurtBoxes))
        index = selected;

    if(index < 0 || index >= boxes.size())
        return NULL;

    return boxes[index];
}

vector<HitBox>* Editor::getHitBoxes(int key) {
    Frame* frame = getKeyFrame(key);

    if(frame) 
        return &frame->hitBoxes;

    return NULL;
}

HitBox* Editor::getHitBox(int index, int key) {
    vector<HitBox>* boxes = getHitBoxes(key);

    if(settings.mode == Mode::HitBoxes && index == -1)
        index = selected;

    if(!boxes)
        return NULL;

    if(index < 0 || index >= boxes->size())
        return NULL;

    return &(*boxes)[index];
}

vector<HurtBox>* Editor::getHurtBoxes(int key) {
    Frame* frame = getKeyFrame(key);

    if(frame) 
        return &frame->hurtBoxes;
        
    return NULL;
}

HurtBox* Editor::getHurtBox(int index, int key) {
    vector<HurtBox>* boxes = getHurtBoxes(key);

    if(settings.mode == Mode::HurtBoxes && index == -1)
        index = selected;

    if(!boxes)
        return NULL;

    if(index < 0 || index >= boxes->size())
        return NULL;

    return &(*boxes)[index];
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

        vector<Player> others = {player};
        player.advanceFrame(others);
        player.advanceEffects(others);

        if(player.state.position.y == 0 && player.doneMove())
            resetPlayer();
    }
}

void Editor::setKeyFrame(int index) {

    if(anim.keyFrames.size() == 0)
        index = -1;
    else
        index = std::clamp(index, 0, (int)anim.keyFrames.size()-1);

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

    // Selections
    if(g::input.pressed(MOUSE_INDEX, sf::Mouse::Button::Left)) {
        bool pickSame = false;

        if(getJoint()) 
            pickSame = Screen::pointInCircle(g::input.mousePosition, {getJoint()->x, getJoint()->y, 8});

        // Pick Joint
        if(!pickSame) {
            vector<Vector2*> joints = getJoints();
            setSelected(-1);

            for(int i = 0; i < joints.size(); i ++) {
                Vector2 joint = g::video.toScreen(*joints[i]);

                if(Screen::pointInCircle(g::input.mousePosition, {joint.x, joint.y, 8})) {
                    setSelected(i);
                    break;
                }
            }
        }
    }

    // Modifications
    if(getJoint()) {
        Skeleton* skele = getJointOwner();

        // Rotate Joint
        if(g::input.held(MOUSE_INDEX, sf::Mouse::Button::Right)) {

            if(skele) {
                skele->rotateJoint(*getJoint(), *getJoint(), -g::input.mouseMove.x * PI / 180);
            }
        }

        // Move Joint
        if(g::input.held(MOUSE_INDEX, sf::Mouse::Button::Left)) {

            if(skele) {
                skele->moveJoint(*getJoint(),
                    {
                        g::input.mouseMove.x * (g::video.camera.w / g::video.getSize().x),
                        -g::input.mouseMove.y * (g::video.camera.h / g::video.getSize().y)
                    }
                );  
            }
        }

        // Draw clickable area
        Vector2 pos = g::video.toScreen(*getJoint());

        sf::CircleShape highlight; 
        highlight.setFillColor({0, 0, 0, 0});
        highlight.setOutlineColor({255, 0, 0});
        highlight.setOutlineThickness(2);
        highlight.setPosition({pos.x - 8, pos.y - 8});
        highlight.setRadius(8);
        g::video.draw(highlight);        
    }
}

void Editor::selectRectangle() {

    // Create Rectangles
    if(g::input.pressed(MOUSE_INDEX, sf::Mouse::Button::Right)) {
        Vector2 pos = g::video.toReal(g::input.mousePosition);
        
        if(settings.mode == Mode::HitBoxes) {
            vector<HitBox>* boxes = getHitBoxes();

            if(boxes) {
                boxes->push_back({pos.x - 25, pos.y + 25, 50, 50});
                setSelected(boxes->size() - 1);
            }

        }else if(settings.mode == Mode::HurtBoxes) {
            vector<HurtBox>* boxes = getHurtBoxes();

            if(boxes) {
                boxes->push_back({pos.x - 25, pos.y + 25, 50, 50});
                setSelected(boxes->size() - 1);
            }
        }
    }

    // Create the drag zones
    Rectangle zone[9];

    if(getBox()) {

        for(int i = 0; i < 9; i ++) {

            if(i == 4)
                continue;

            Vector2 pos {
                getBox()->x + (getBox()->w/2) * (i % 3),
                getBox()->y - (getBox()->h/2) * (i / 3)
            };

            pos = g::video.toScreen(pos);

            zone[i].x = pos.x - 4;
            zone[i].y = pos.y - 4;
            zone[i].w = 8;
            zone[i].h = 8;

            sf::RectangleShape rect = zone[i];
            rect.setFillColor(sf::Color::Transparent);
            rect.setOutlineColor(sf::Color::White);
            rect.setOutlineThickness(1);

            g::video.draw(rect);
        }
    }

    // Selection
    if(g::input.pressed(MOUSE_INDEX, sf::Mouse::Button::Left)) {
        bool pickSame = false;

        if(getBox()) {

            // Check each drag zone to determine selection
            if(Screen::pointInRectangle(g::input.mousePosition, g::video.toScreen(*getBox()))) {
                setDragZone(4);
                pickSame = true;
            }

            for(int i = 0; i < 9; i ++) {

                if(i == 4)
                    continue;

                if(Screen::pointInRectangle(g::input.mousePosition, zone[i])) {
                    setDragZone(i);
                    pickSame = true;
                    break;
                }
            }
        }

        // Make new selection
        if(!pickSame) {
            vector<Rectangle*> boxes = getBoxes();
            setSelected(-1);

            for(int i = 0; i < boxes.size(); i ++) {

                if(Screen::pointInRectangle(g::input.mousePosition, g::video.toScreen(*boxes[i]))) {
                    setSelected(i);
                    break;
                }
            }
        }
    }

    // Modifications
    if(getBox()) {

        if(g::input.held(MOUSE_INDEX, sf::Mouse::Button::Left)) {
            vector<Rectangle*> boxes = getBoxes();
            Vector2 mov = g::input.mouseMove * (g::video.camera.w / g::video.getSize().x);

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

        if(g::input.pressed(KEYBOARD_INDEX, sf::Keyboard::Delete)) {

            if(settings.mode == Mode::HitBoxes && getHitBox()) {
                getHitBoxes()->erase(getHitBoxes()->begin() + selected);

            }else if(settings.mode == Mode::HurtBoxes && getHurtBoxes()) {
                getHurtBoxes()->erase(getHurtBoxes()->begin() + selected);
            }
            setSelected(-1);
        }          
    }
}