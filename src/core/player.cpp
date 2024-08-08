#include <fstream>
#include <cmath>
#include <json.hpp>

#include "render_instance.h"
#include "input_interpreter.h"
#include "save.h"
#include "math.h"
#include "save.h"
#include "player.h"

void Player::draw() {
    Skeleton pose = getSkeleton();

    for(int i = 0; i < pose.jointCount; i ++) 
        pose.joints[i] = g::video.camera.getScreen(pose.joints[i]);

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
    if(local_id == -1)
        return out;

    Button::Config buttonConfig = g::save.getButtonConfig(local_id);

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

void Player::advanceFrame(Button::Flag in, vector<Player> others) {

    Player& opponent = others[state.target];

    // Add input to the button history
    for(int i = Button::History - 1; i > 0; i --) {
        state.button[i] = state.button[i - 1];
    }
    state.button[0] = in;   

    // Hitstop
    if(state.hitStop > 0)
        state.hitStop --;

    if(state.hitStop < 0)
        state.hitStop ++;

    // Check if taken any damage and set the HitStop
    int dmg1 = state.accDamage;
    int dmg2 = opponent.state.accDamage;

    if(dmg1 > 0 || dmg2 > 0) {
        state.hitStop = std::max(dmg1, dmg2);

        // If this player is damaged signal with a negative hitStop
        if(dmg1 > 0)
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

    // Damage, only be hit once per keyframe
    // Opponent doing a new move can reset the hitKeyFrame
    if(opponent.getKeyFrame() != state.hitKeyFrame)
        state.hitKeyFrame = -1;

    bool damageValid = (state.hitKeyFrame != opponent.getKeyFrame());

    if(damageValid) {

        // Check HurtBox <-> HitBox collisions, for only one match
        bool found = false;

        for(auto& hurt : getHurtBoxes()) {
            for(auto& hit : opponent.getHitBoxes()) {

                if(Real::rectangleInRectangle(hurt, hit)) {

                    // Set what keyframe you were hit on
                    state.hitKeyFrame = opponent.getKeyFrame();        

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

                        if(inMove(Move::Stand) || inMove(Move::StandBlock)){
                            setMove(Move::StandBlock);
                            state.stun = hit.blockStun;
                        
                        }else{
                            setMove(Move::CrouchBlock);
                            state.stun = hit.blockStun;
                        }

                    }else{
                        dealDamage(hit.damage);
                        state.velocity = hit.force;

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
                    }

                    found = true;
                    break;
                }
            }

            if(found)
                break;
        }
    }

    // Hit/Block Stun finish
    if((inMove(Move::StandCombo) || inMove(Move::StandBlock)) && state.stun == 0) 
        setMove(Move::Stand);

    if((inMove(Move::CrouchCombo) || inMove(Move::CrouchBlock)) && state.stun == 0) 
        setMove(Move::Crouch);

    if(inMove(Move::GetUp) && doneMove())
        setMove(Move::Stand);

    // Special Move Check
    {
        string motion = "";
        for(int i = 0; i < 30; i ++) 
            motion = getMotion(i) + motion;

        int best = -1;

        for(int i = Move::Custom00; i < Move::Total; i ++) {
            string str = config.motions[i];
            int u = 0;
            int v = 0;
            bool match = false;

            // Check if in the required stance
            Animation* anim = g::save.getAnimation(config.moves[i]);

            if(!anim)
                continue;

            bool good[] = {
                anim->inCrouch && inMove(Move::Crouch),
                anim->inStand && inMove(Move::Stand),
                anim->inJump && inMove(Move::Jump)
            };

            if(!good[0] && !good[1] && !good[2])
                continue;

            // Check if the last motion matches between the strings
            string lastMotion[2] {"", ""};
            string lastButton[2] {motion.substr(0), str.substr(0)};

            for(int j = motion.size() - 1; j >= 0; j --) {

                if(motion[j] >= '0' && motion[j] <= '9'){
                    lastMotion[0] = motion[j];
                    lastButton[0] = motion.substr(j+1);
                    break;
                }
            }

            for(int j = str.size() - 1; j >= 0; j --) {
                
                if(str[j] >= '0' && str[j] <= '9'){
                    lastMotion[1] = str[j];
                    lastButton[1] = str.substr(j+1);
                    break;
                }
            }

            if(!((lastMotion[0] == lastMotion[1] || lastMotion[1] == "") && lastButton[0] == lastButton[1])) {
                continue;
            }

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

        if(best != -1)
            setMove(best);
    }

    // Custom move finish
    if(state.moveIndex >= Move::Custom00 && state.moveIndex < Move::Total && doneMove()) {

        if(state.position.y <= 0)
            setMove(Move::Stand);
    }

    // Movement
    state.velocity += getFrame().impulse;
    state.position += state.velocity;

    if(state.position.y <= 0) {
        state.position.y = 0;
        state.velocity.y = 0;
        state.velocity.x /= 1.1;

        if(std::abs(state.velocity.x) < 0.25)
            state.velocity.x = 0;

        if((inMove(Move::JumpCombo) || inMove(Move::KnockDown)))
            setMove(Move::GetUp);

        if(inMove(Move::Stand) || inMove(Move::Crouch))
            state.velocity.x = 0;

    }else{
        state.velocity.y -= 0.25;
    }

    if(inMove(Move::Jump) && state.position.y <= 0 && doneMove()) {
        setMove(Move::Stand);
    }

    if(inMove(Move::Stand) || inMove(Move::Crouch)) {

        // If on ground look in direction of opponent
        state.side = (state.position.x < opponent.state.position.x) ? 1 : -1;

        // Reset to grounded
        state.velocity = {0, 0};

        // Standing
        if(getSOCD().y == 0) {

            if(getSOCD().x == state.side){
                setMove(Move::WalkForwards, true);

            }else if(getSOCD().x == -state.side){
                setMove(Move::WalkBackwards, true);

            }else {
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

    // Collision checks
    if((state.position - opponent.state.position).getDistance() < 25) {
        state.position.x = opponent.state.position.x + 25 * opponent.state.side;
    }

    // Clamp look to side, no 180 degree head turns
    if(state.target != -1) {
        Skeleton myPose = getSkeleton();
        Skeleton opPose = others[state.target].getSkeleton();

        state.look = (opPose.head - myPose.head).getAngle();

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
    } 
}

void Player::dealDamage(int dmg) {
    state.health -= dmg;
    state.accDamage += dmg;

    if(state.health < 0)
        state.health = 0;
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