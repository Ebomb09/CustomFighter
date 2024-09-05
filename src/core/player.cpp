#include "player.h"
#include "move.h"
#include "render_instance.h"
#include "input_interpreter.h"
#include "save.h"
#include "math.h"
#include "save.h"
#include "audio.h"

#include <fstream>
#include <cmath>
#include <json.hpp>
#include <iostream>

void Player::draw() {
    Skeleton pose = getSkeleton();

    // HitStop shake
    if(state.hitStop < 0) {

        for(int i = 0; i < pose.jointCount; i ++) 
            pose.joints[i].x += std::sin(state.hitStop) * 2;
    }

    pose.draw(getClothes(), state.look);
}

Button::Flag Player::readInput() {
    Button::Flag out;

    // Get the local save button config
    if(seatIndex == -1)
        return out;

    Button::Config buttonConfig = g::save.getButtonConfig(seatIndex);

    out.A       = g::input.keyPressed[buttonConfig.A];
    out.B       = g::input.keyPressed[buttonConfig.B];
    out.C       = g::input.keyPressed[buttonConfig.C];
    out.D       = g::input.keyPressed[buttonConfig.D];   
    out.Taunt   = g::input.keyPressed[buttonConfig.Taunt];   

    // Any attack button is pressed then retrigger all held attack buttons
    if(out.A || out.B || out.C || out.D || out.Taunt) {
        out.A       = g::input.keyHeld[buttonConfig.A];
        out.B       = g::input.keyHeld[buttonConfig.B];        
        out.C       = g::input.keyHeld[buttonConfig.C];
        out.D       = g::input.keyHeld[buttonConfig.D];   
        out.Taunt   = g::input.keyHeld[buttonConfig.Taunt];         
    }

    // Movement keys always held down
    out.Up      = g::input.keyHeld[buttonConfig.Up];
    out.Down    = g::input.keyHeld[buttonConfig.Down];
    out.Left    = g::input.keyHeld[buttonConfig.Left];
    out.Right   = g::input.keyHeld[buttonConfig.Right];

    return out;
}

void Player::advanceFrame(vector<Player> others) {

    // Find other currently tagged in player
    int target = getTarget(others);

    // Hitstop
    if(state.hitStop > 0)
        state.hitStop --;

    if(state.hitStop < 0)
        state.hitStop ++;

    // Add input to the button history
    for(int i = Button::History - 1; i > 0; i --) 
        state.button[i] = state.button[i - 1];

    state.button[0] = in;

    // Check if any damage was dealt and set the HitStop
    int dmg = 0;

    for(Player& other : others) 
        dmg = std::max(dmg, other.state.accDamage);

    if(dmg > 0) {
        state.hitStop = dmg;

        // If this player is damaged signal with a negative hitStop
        if(state.accDamage > 0)
            state.hitStop = -state.hitStop;
    }
    state.accDamage = 0;

    // Exit early if in HitStop
    if(state.hitStop != 0) 
        return;

    // Increment frames
    state.moveFrame ++;

    if(state.stun > 0)
        state.stun --;

    if(state.hitKeyFrame > 0)
        state.hitKeyFrame --;

    // Check Hit/Hurtbox Collisions
    HitBox hit = getCollision(others);

    if(hit.damage > 0) {  
        // Block if input in the opposite direction of opponent           
        bool block = false;

        if(inMove(Move::Stand) || inMove(Move::StandBlock) ||
            inMove(Move::Crouch) || inMove(Move::CrouchBlock)) {

            if((state.side == 1 && getSOCD().x < 0) || (state.side == -1 && getSOCD().x > 0)) {
                block = true;
            }
        }

        if(block) {

            // Push back
            state.velocity.x = hit.force.x;
            state.stun = hit.blockStun;
            state.accDamage = hit.blockStun;

            if(inMove(Move::Stand) || inMove(Move::StandBlock)){
                setMove(Move::StandBlock);

            }else{
                setMove(Move::CrouchBlock);
            }
            g::audio.playSound(g::save.getSound("block"));

        }else{
            dealDamage(hit.damage);
            state.velocity.x += hit.force.x;
            state.velocity.y = hit.force.y;

            if(hit.knockdown) {
                setMove(Move::KnockDown);

            }else if(state.position.y > 0 || state.velocity.y > 0){
                setMove(Move::JumpCombo);

            }else if(inMove(Move::Crouch) || inMove(Move::CrouchBlock) || inMove(Move::CrouchCombo)){
                setMove(Move::CrouchCombo);
                state.stun = hit.hitStun;

            }else{
                setMove(Move::StandCombo);
                state.stun = hit.hitStun;
            }
            g::audio.playSound(g::save.getSound("hurt"));
        }
    }

    // Special Move Check
    if(getFrame().cancel && taggedIn(others)){
        string motion = "";
        for(int i = 0; i < 30; i ++) 
            motion = getMotion(i) + motion;

        int best = -1;

        for(int i = Move::Custom00; i < Move::Total; i ++) {
            string str = config.motions[i];
            int u = 0;
            int v = 0;
            bool match = false;

            // Validate both animations
            Animation* currAnim = g::save.getAnimation(config.moves[state.moveIndex]);
            Animation* nextAnim = g::save.getAnimation(config.moves[i]);

            if(!currAnim || !nextAnim)
                continue;

            // Check valid to cancel into the next anim
            if(!nextAnim->from[currAnim->category] && nextAnim->customFrom != currAnim->name) 
                continue;

            // Check if the last motion matches between the strings
            string lastMotion[2] {"", ""};

            for(int j = motion.size() - 1; j >= 0; j --) {

                if(motion[j] >= '0' && motion[j] <= '9'){
                    lastMotion[0] = motion[j];
                    break;
                }
            }

            for(int j = str.size() - 1; j >= 0; j --) {
                
                if(str[j] >= '0' && str[j] <= '9'){
                    lastMotion[1] = str[j];
                    break;
                }
            }

            if(lastMotion[0] != lastMotion[1] && lastMotion[1].size() > 0)
                continue;

            // Scan the motion strings for matching inputs
            while(u < str.size() && v < motion.size()) {

                if(str[u] == motion[v]) {
                    u ++;
                    v ++;

                    if(u == str.size()) {
                        match = true;
                        break;
                    }

                }else {
                    v ++;
                }
            }

            // A correctly matching move was found
            if(match) {

                if(best == -1)
                    best = i;

                else if(config.motions[i].size() > config.motions[best].size()) 
                    best = i;
            }
        }

        // Force reset of the moveFrame, so cancels into the same move work
        if(best != -1) {
            state.moveFrame = 0;
            setMove(best);

            // Clear input history to prevent misinputs
            for(int i = 0; i < Button::History; i ++)
                state.button[i] = (Button::Flag)0;
        }
    }

    // Movement
    state.velocity += getFrame().impulse;
    state.position += state.velocity;

    // Collision checks
    if(target != -1) {

        // Unstick players
        while((state.position - others[target].state.position).getDistance() < 25) {

            if(state.position.x < others[target].state.position.x)
                state.position.x -= 1;

            else if(state.position.x == others[target].state.position.x)
                state.position.x += (state.position.x < 0) ? 1 : -1;

            else
                state.position.x += 1;
        }   
    }

    if(taggedIn(others)) {
        // Clamp position within the camera
        state.position.x = std::clamp(
            state.position.x, 
            g::video.camera.x + 16,
            g::video.camera.x + g::video.camera.w - 16
            );

        // Clamp position within stage bounds
        state.position.x = std::clamp(
            state.position.x, 
            StageBounds.x + 16, 
            StageBounds.x + StageBounds.w - 16
            );  
    }
  
    if(state.position.y <= 0) {
        state.position.y = 0;
        state.velocity.y = 0;

        // Don't move while in full movement control
        if(inMove(Move::Stand) || inMove(Move::Crouch) || inMove(Move::TagIn) || inMove(Move::TagOut) ||
            inMove(Move::Jump)) {
            state.velocity.x = 0;

        // Always slide during some custom move
        } else {

            if(state.velocity.x > 0)
                state.velocity.x = std::clamp(state.velocity.x - 0.25f, 0.f, state.velocity.x);

            if(state.velocity.x < 0)
                state.velocity.x = std::clamp(state.velocity.x + 0.25f, state.velocity.x, 0.f);    
        }

        // Custom move commit to move until on ground
        if(inMove(Move::Custom00) && doneMove())
            setMove(Move::Stand);

        // Anticipate the jump startup and only return once animation is done
        if(inMove(Move::Jump) && doneMove())
            setMove(Move::Stand);

        // Taunt cancel
        if(inMove(Move::Taunt) && doneMove()) {
            setMove(Move::Stand);
            state.tagCount ++;
        }

        // Player stay down once defeated
        if(state.health <= 0) {
            setMove(Move::KnockDown);

            if(doneMove()) 
                state.tagCount = -1;
        }

        // Tag in / out animations
        if((inMove(Move::TagIn) || inMove(Move::TagOut)) && doneMove())
            setMove(Move::Stand);    

        // Hit States
        if(inMove(Move::JumpCombo)) 
            setMove(Move::GetUp);

        if(inMove(Move::KnockDown) && state.health > 0)
            setMove(Move::GetUp);

        if((inMove(Move::StandCombo) || inMove(Move::StandBlock)) && state.stun == 0) 
            setMove(Move::Stand);

        if((inMove(Move::CrouchCombo) || inMove(Move::CrouchBlock)) && state.stun == 0) 
            setMove(Move::Crouch);

        if(inMove(Move::GetUp) && doneMove())
            setMove(Move::Stand);

        // Controls
        if(taggedIn(others)) {

            // Movement options
            if(inMove(Move::Stand) || inMove(Move::Crouch)) {

                // If on ground look in direction of opponent
                if(target != -1)
                    state.side = (state.position.x < others[target].state.position.x) ? 1 : -1;

                // Standing
                if(getSOCD().y == 0) {

                    if(in.Taunt) {
                        setMove(Move::Taunt);
            
                    }else if(getSOCD().x == state.side){
                        setMove(Move::WalkForwards, true);

                    }else if(getSOCD().x == -state.side){
                        setMove(Move::WalkBackwards, true);

                    }else{
                        setMove(Move::Stand, true);
                    }

                // Jumping
                }else if(getSOCD().y > 0) {

                    if(getSOCD().x == state.side){
                        setMove(Move::JumpForwards);

                    }else if(getSOCD().x == -state.side){
                        setMove(Move::JumpBackwards);

                    }else {
                        setMove(Move::Jump);
                    }

                // Crouching
                }else if(getSOCD().y < 0) {
                    setMove(Move::Crouch, true);
                }      
            }

        // Always prepare to be tagged in
        }else if(state.health > 0){
            bool prepare = false;
            int curr = -1;

            // Find currently tagged in member
            for(int i = 0; i < others.size(); i ++) {
                Player& ply = others[i];

                if(ply.team == team && ply.gameIndex != gameIndex && ply.taggedIn(others)) {
                    curr = i;
                    prepare = ply.inMove(Move::Taunt) || ply.state.health <= 0;
                }
            }

            // Move in
            if(prepare) {

                // Run in until up to player
                if((state.side == 1 && state.position.x < others[curr].state.position.x + 16) ||
                    (state.side == -1 && state.position.x > others[curr].state.position.x - 16)) {

                    setMove(Move::TagIn, true);

                // Reset to stand once arrived
                }else{
                    setMove(Move::Stand);
                }

            // In camera area, tag out
            }else if(state.position.x > g::video.camera.x - 16 && state.position.x < g::video.camera.x + g::video.camera.w + 16) {
                setMove(Move::TagOut, true);

            // Not in camera area, side line and prepare to move in
            }else if(curr != -1) {
                state.side = others[curr].state.side;

                // Just off screen
                state.position = {
                    (state.side == 1) ? g::video.camera.x - 64 : g::video.camera.x + g::video.camera.w + 64,
                    0
                };
                state.velocity = {0, 0};
            }
        }

    }else{
        state.velocity.y -= 0.25;
    }

    // Look at the target while still alive
    if(state.health > 0) {

        if(target != -1) {
            Skeleton myPose = getSkeleton();
            Skeleton opPose = others[target].getSkeleton();

            state.look = (opPose.head - myPose.head).getAngle();

            // Clamp look to side, no 180 degree head turns
            if(state.side == 1) {
                
                if(state.look > PI * 1/4.f)
                    state.look = PI * 1/4.f;

                if(state.look < -PI * 1/4.f)
                    state.look = -PI * 1/4.f;

            }else {

                if(state.look < PI * 3/4.f && state.look >= 0)
                    state.look = PI * 3/4.f;

                if(state.look > -PI * 3/4.f && state.look <= 0)
                    state.look = -PI * 3/4.f;
            }   

        // Default head position based on side
        }else {
            state.look = (state.side == 1) ? 0 : PI;
        }
    }

    // Reset input
    in = Button::Flag();
}

void Player::dealDamage(int dmg) {
    state.health -= dmg;
    state.accDamage += dmg;

    if(state.health < 0)
        state.health = 0;
}

int Player::getTarget(vector<Player> others) {

    if(!taggedIn(others))
        return -1;

    for(int i = 0; i < others.size(); i ++) {

        if(others[i].team != team && others[i].taggedIn(others)) {
            return i;
        }
    }
    return -1;
}

string Player::getMotion(int index) {
    string out = "";

    Vector2 socd = getSOCD(index);
    socd.x *= state.side;

    out += ('5' + (int)socd.x + (int)socd.y * 3);

    if(state.button[index].A)
        out += (out.size() == 1) ? "A" : "+A";

    if(state.button[index].B)
        out += (out.size() == 1) ? "B" : "+B";

    if(state.button[index].C)
        out += (out.size() == 1) ? "C" : "+C";

    if(state.button[index].D)
        out += (out.size() == 1) ? "D" : "+D";

    if(state.button[index].Taunt)
        out += (out.size() == 1) ? "P" : "+P";

    return out;
}

bool Player::inMove(int move) {
    int current = state.moveIndex;

    // Some moves are synonyms for each other
    switch(current) {

    case Move::WalkForwards:
    case Move::WalkBackwards:
        current = Move::Stand;
        break;

    case Move::JumpForwards:
    case Move::JumpBackwards:
        current = Move::Jump;
        break;
    }

    if(current >= Move::Custom00 && current < Move::Total)
        current = Move::Custom00;

    return current == move;
}

void Player::setMove(int move, bool loop) {

    if(state.moveIndex != move) {
        state.moveIndex = move;
        state.moveFrame = 0;

    }else if(loop) {
        Animation* anim = g::save.getAnimation(config.moves[state.moveIndex]);

        if(state.moveFrame >= anim->getFrameCount())
            state.moveFrame = 0;
    }
}

bool Player::doneMove() {
    Animation* anim = g::save.getAnimation(config.moves[state.moveIndex]);
    return (state.moveFrame >= anim->getFrameCount());
}

bool Player::taggedIn(vector<Player> others) {

    // Indicates out of the game
    if(state.tagCount < 0)
        return false;

    for(auto& ply : others) {

        if(ply.gameIndex == gameIndex || ply.state.tagCount < 0)
            continue;

        if(ply.team == team) {

            if(ply.state.tagCount == state.tagCount)
                return gameIndex < ply.gameIndex;
            else
                return (state.tagCount < ply.state.tagCount);
        }
    }
    return true;
}

HitBox Player::getCollision(vector<Player> others) {

    if(!taggedIn(others))
        return {};

    if(state.hitKeyFrame != 0)
        return {};

    for(auto& ply : others) {

        if(ply.team == team)
            continue;

        // Check HurtBox <-> HitBox collisions, for only one match
        for(auto& hit : ply.getHitBoxes()) {
            for(auto& hurt : getHurtBoxes()) {

                if(Real::rectangleInRectangle(hurt, hit)) {
                    state.hitKeyFrame = ply.getFrame().duration;
                    return hit;
                }
            }
        }            
    }        
    return {};
}

int Player::getKeyFrame() {
    Animation* anim = g::save.getAnimation(config.moves[state.moveIndex]);

    int key = 0;
    int time = 0;

    for(int i = 0; i < anim->keyFrames.size(); i ++) {

        if(state.moveFrame >= time)
            key = i;

        time += anim->keyFrames[i].duration;
    }
    return key;
}

Frame Player::getFrame() {
    Animation* anim = g::save.getAnimation(config.moves[state.moveIndex]);

    Frame frame;

    if(anim) {
        frame = anim->getFrame(state.moveFrame);

        // Correct directional attributes of the frame
        frame.impulse.x *= state.side;

        for(auto& box : frame.hitBoxes) {

            if(state.side == -1)
                box.x = -box.x - box.w;

            box.x += state.position.x;
            box.y += state.position.y;

            box.force.x *= state.side;            
        }

        for(auto& box : frame.hurtBoxes) {

            if(state.side == -1)
                box.x = -box.x - box.w;

            box.x += state.position.x;
            box.y += state.position.y;        
        }

        for(int i = 0; i < frame.pose.jointCount; i ++) {
            frame.pose.joints[i].x *= state.side;
            frame.pose.joints[i] += state.position;
        }

        // Flip draw order array
        if(state.side == -1) {
            for(int i = 0; i < SkeletonDrawOrder::Total / 2; i ++) 
                std::swap(frame.pose.order[i], frame.pose.order[SkeletonDrawOrder::Total - 1 - i]);
        }
    }
    return frame;
}

Skeleton Player::getSkeleton() {
    return getFrame().pose;
}

vector<HitBox> Player::getHitBoxes() {
    return getFrame().hitBoxes;
}

vector<HurtBox> Player::getHurtBoxes() {
    return getFrame().hurtBoxes;   
}

vector<Clothing*> Player::getClothes() {
    vector<Clothing*> out;

    // Implied clothing... skin
    out.push_back(g::save.getClothing("realistic"));

    for(auto& it : config.clothes)
        out.push_back(g::save.getClothing(it));

    return out;
}

Vector2 Player::getSOCD(int index) {
    Vector2 mov;

    if(state.button[index].Right)
        mov.x += 1;

    if(state.button[index].Left)
        mov.x -= 1;

    if(state.button[index].Up)
        mov.y += 1;

    if(state.button[index].Down)
        mov.y -= 1;

    return mov;
}

void Player::Config::loadFromText(string str) {
    nlohmann::json json;

    try {
        json = nlohmann::json::parse(str);

    }catch(nlohmann::json::exception e) {
        return;
    }

    if(json["clothes"].is_array()) {
        clothes.clear();

        for(int i = 0; i < json["clothes"].size(); i ++) 
            clothes.push_back(json["clothes"][i]);
    }

    if(json["moves"].is_array()) {

        for(int i = 0; i < json["moves"].size(); i ++) 
            moves[i] = json["moves"][i];
    }

    if(json["motions"].is_array()) {

        for(int i = 0; i < json["motions"].size(); i ++) 
            motions[i] = json["motions"][i];   
    }       
}

string Player::Config::saveToText() {
    nlohmann::json json;
    json["clothes"] = clothes;
    json["moves"] = moves;   
    json["motions"] = motions;     

    return json.dump();
}

void Player::Config::loadFromFile(std::string path) {
    std::fstream file(path, std::fstream::in);

    if(!file.good()) {
        file.close();
        return;
    }

    std::stringstream stream;
    stream << file.rdbuf();
    string str = stream.str();
    loadFromText(str);  
}

void Player::Config::saveToFile(string path) {
    std::fstream file(path, std::fstream::out | std::fstream::trunc);

    if(!file.good()) {
        file.close();
        return;
    }

    file << saveToText();
    file.close();   
}

Rectangle Player::getRealBoundingBox() {
    // Get the position of the skeleton
    Skeleton pose = getSkeleton();
    Vector2 min;
    Vector2 max;

    min = {std::min(min.x, 0.f), std::min(min.y, 0.f)};
    max = {std::max(max.x, 0.f), std::max(max.y, 0.f)};

    for(int i = 0; i < pose.jointCount; i ++) {
        min = {std::min(min.x, pose.joints[i].x), std::min(min.y, pose.joints[i].y)};
        max = {std::max(max.x, pose.joints[i].x), std::max(max.y, pose.joints[i].y)};
    }
    return {min.x, max.y, max.x - min.x, max.y - min.y};
}

Rectangle Player::getScreenBoundingBox() {
    return g::video.camera.getScreen(getRealBoundingBox());
}