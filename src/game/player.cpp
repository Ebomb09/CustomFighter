#include <cmath>

#include "core/input_interpreter.h"
#include "core/math.h"
#include "core/save.h"

#include "player.h"

Player::Player() {
    State& state = getState(0);
    state.health = 100;
    state.stun = 0;
    state.hitStop = 0;
    state.moveIndex = Move::Stand;
    state.moveFrame = 0;
    state.config = &config;

    config.clothes.push_back("realistic");
    config.clothes.push_back("shirt");

    for(int i = 0; i < Button::Total; i ++)
        config.button[i] = 0;

    for(int i = 0; i < Button::Total; i ++) 
        state.button[i] = false;
}

void Player::draw(int gameFrame) {
    State& state = getState(gameFrame);
    State& opState = opponent->getState(gameFrame);

    Skeleton pose = state.getSkeleton();
    Skeleton opPose = opState.getSkeleton();

    for(int i = 0; i < pose.jointCount; i ++) {
        pose.joints[i] = g::video.camera.getScreen(pose.joints[i]);
        opPose.joints[i] = g::video.camera.getScreen(opPose.joints[i]);
    }

    float look = (opPose.head - pose.head).getAngle();

    // Clamp look to side, no 180 degree head turns
    if(state.side == 1) {

        if(look > PI * 1/4.f)
            look = PI * 1/4.f;

        if(look < -PI * 1/4.f)
            look = -PI * 1/4.f;

    }else {

        if(look < PI * 3/4.f && look >= 0)
            look = PI * 3/4.f;

        if(look > -PI * 3/4.f && look <= 0)
            look = -PI * 3/4.f;
    }

    // HitStop shake
    if(state.hitStop < 0) {

        for(int i = 0; i < pose.jointCount; i ++) 
            pose.joints[i].x += std::sin(state.hitStop) * 2;
    }

    pose.draw(getClothes(), look);
}

void Player::readInput(int gameFrame) {
    State& state = getState(gameFrame);

    // Reset buttons
    for(int i = 0; i < Button::Total; i ++)
        state.button[i] = false;

    // Any attack button is pressed then retrigger all held attack buttons
    for(int i = Button::A; i <= Button::Taunt; i ++) {

        if(g::input.keyPressed[config.button[i]]) {

            for(int j = Button::A; j <= Button::Taunt; j ++) 
                state.button[j] = g::input.keyHeld[config.button[j]];   

            break;
        }
    }

    // Get inputs via config and interpreter
    for(int i = Button::Up; i <= Button::Right; i ++)
        state.button[i] = g::input.keyHeld[config.button[i]];
}

Player::State& Player::getState(int gameFrame, bool copy) {

    if(gameFrame >= history.size()) {

        if(gameFrame != 0) {

            for(int i = history.size(); i <= gameFrame; i ++) {
                State state = history[i - 1];

                for(int j = 0; j < Button::Total; j ++)
                    state.button[j] = false;

                history.push_back(state);
            }

        // Default State push
        }else {
            history.push_back(State());
        }
    }

    // Copy over everything except the buttons
    if(copy && gameFrame != 0) {
        State temp = getState(gameFrame);
        history[gameFrame] = history[gameFrame-1];

        for(int i = 0; i < Button::Total; i ++)
            history[gameFrame].button[i] = temp.button[i];
    }

    return history[gameFrame];
}

Player::State& Player::getNextState(int gameFrame) {

    // Get last frame to compute next
    State state = getState(gameFrame);
    State opState = opponent->getState(gameFrame);

    // Hitstop
    if(state.hitStop > 0)
        state.hitStop --;

    if(state.hitStop < 0)
        state.hitStop ++;

    if(gameFrame > 0) {
        int diff1 = getState(gameFrame - 1).health - state.health;
        int diff2 = opponent->getState(gameFrame - 1).health - opState.health;        

        if(diff1 > 0 || diff2 > 0) {
            state.hitStop = std::max(diff1, diff2);

            // If this player is damaged signal with a negative hitStop
            if(diff1 > 0)
                state.hitStop = -state.hitStop;
        }
    }

    // Exit early if in HitStop
    if(state.hitStop != 0) {

        // Return a reference to our completion
        getState(gameFrame + 1) = state;
        return getState(gameFrame + 1);
    }

    // Increment frames
    state.moveFrame ++;

    if(state.stun > 0)
        state.stun --;

    // Damage, only be hit once per keyframe
    if(gameFrame > 0) { 
        State opLastState = opponent->getState(gameFrame - 1);

        // Opponent doing a new move can reset the hitKeyFrame
        if(opState.moveIndex != opLastState.moveIndex)
            state.hitKeyFrame = -1;
    }

    bool damageValid = (state.hitKeyFrame != opState.getKeyFrame());

    if(damageValid) {

        // Check HurtBox <-> HitBox collisions, for only one match
        bool found = false;

        for(auto& hurt : state.getHurtBoxes()) {
            for(auto& hit : opState.getHitBoxes()) {

                if(Real::rectangleInRectangle(hurt, hit)) {

                    // Set what keyframe you were hit on
                    state.hitKeyFrame = opState.getKeyFrame();        

                    // Block if input in the opposite direction of opponent           
                    bool block = false;

                    if(state.inMove(Move::Stand) || state.inMove(Move::StandBlock) ||
                        state.inMove(Move::Crouch) || state.inMove(Move::CrouchBlock)) {

                        if((state.side == 1 && state.getSOCD().x < 0) || (state.side == -1 && state.getSOCD().x > 0)) {
                            block = true;
                        }
                    }

                    if(block) {

                        // Push back
                        state.velocity.x = hit.force.x;

                        if(state.inMove(Move::Stand) || state.inMove(Move::StandBlock)){
                            state.setMove(Move::StandBlock);
                            state.stun = hit.blockStun;
                        
                        }else{
                            state.setMove(Move::CrouchBlock);
                            state.stun = hit.blockStun;
                        }

                    }else{
                        state.health -= hit.damage;
                        state.velocity = hit.force;

                        if(hit.knockdown) {
                            state.setMove(Move::KnockDown);

                        }else if(state.position.y > 0 || state.velocity.y > 0){
                            state.setMove(Move::JumpCombo);

                        }else if(state.inMove(Move::Crouch) || state.inMove(Move::CrouchBlock) || state.inMove(Move::CrouchCombo)){
                            state.setMove(Move::CrouchCombo);
                            state.stun = hit.hitStun;

                        }else{
                            state.setMove(Move::StandCombo);
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
    if((state.inMove(Move::StandCombo) || state.inMove(Move::StandBlock)) && state.stun == 0) 
        state.setMove(Move::Stand);

    if((state.inMove(Move::CrouchCombo) || state.inMove(Move::CrouchBlock)) && state.stun == 0) 
        state.setMove(Move::Crouch);

    if(state.inMove(Move::GetUp) && state.doneMove())
        state.setMove(Move::Stand);

    // Special Move Check
    {
        string motion = "";
        for(int i = 0; i < 30; i ++) {

            if(gameFrame - i < 0)
                break;

            motion = getState(gameFrame - i).getMotion() + motion;
        }

        int best = -1;

        for(int i = Move::Custom00; i < Move::Total; i ++) {
            string str = config.motion[i];
            int u = 0;
            int v = 0;
            bool match = false;

            // Check if in the required stance
            Animation* anim = g::save.getAnimation(config.move[i]);

            if(!anim)
                continue;

            bool good[] = {
                anim->inCrouch && state.inMove(Move::Crouch),
                anim->inStand && state.inMove(Move::Stand),
                anim->inJump && state.inMove(Move::Jump)
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

                else if(config.motion[i].size() > config.motion[best].size()) 
                    best = i;
            }
        }

        if(best != -1)
            state.setMove(best);
    }

    // Custom move finish
    if(state.moveIndex >= Move::Custom00 && state.moveIndex < Move::Total && state.doneMove()) {

        if(state.position.y <= 0)
            state.setMove(Move::Stand);
    }

    // Movement
    state.velocity += state.getFrame().impulse;
    state.position += state.velocity;

    if(state.position.y <= 0) {
        state.position.y = 0;
        state.velocity.y = 0;
        state.velocity.x /= 1.1;

        if(std::abs(state.velocity.x) < 0.25)
            state.velocity.x = 0;

        if((state.inMove(Move::JumpCombo) || state.inMove(Move::KnockDown)))
            state.setMove(Move::GetUp);

        if(state.inMove(Move::Stand) || state.inMove(Move::Crouch))
            state.velocity.x = 0;

    }else{
        state.velocity.y -= 0.25;
    }

    if(state.inMove(Move::Jump) && state.position.y <= 0 && state.doneMove()) {
        state.setMove(Move::Stand);
    }

    if(state.inMove(Move::Stand) || state.inMove(Move::Crouch)) {

        // If on ground look in direction of opponent
        state.side = (state.position.x < opState.position.x) ? 1 : -1;

        // Reset to grounded
        state.velocity = {0, 0};

        // Standing
        if(state.getSOCD().y == 0) {

            if(state.getSOCD().x == state.side){
                state.setMove(Move::WalkForwards, true);

            }else if(state.getSOCD().x == -state.side){
                state.setMove(Move::WalkBackwards, true);

            }else {
                state.setMove(Move::Stand, true);
            }

        // Jumping
        }else if(state.getSOCD().y > 0) {

            if(state.getSOCD().x == state.side){
                state.setMove(Move::JumpForwards);

            }else if(state.getSOCD().x == -state.side){
                state.setMove(Move::JumpBackwards);

            }else {
                state.setMove(Move::Jump);
            }

        // Crouching
        }else if(state.getSOCD().y < 0) {
            state.setMove(Move::Crouch, true);
        }      
    }

    // Collision checks
    if((state.position - opState.position).getDistance() < 25) {
        int side = (state.position.x < opState.position.x) ? 1 : -1;
        state.position.x = opState.position.x + 25 * -side;
    }

    // Return a reference to our completion
    getState(gameFrame + 1) = state;
    return getState(gameFrame + 1);
}

string Player::State::getMotion() {
    string out = "";
    Vector2 socd = getSOCD();
    socd.x *= side;

    out += ('5' + (int)socd.x + (int)socd.y * 3);

    if(button[Button::A])
        out += (out.size() == 1) ? "A" : "+A";

    if(button[Button::B])
        out += (out.size() == 1) ? "B" : "+B";

    if(button[Button::C])
        out += (out.size() == 1) ? "C" : "+C";

    if(button[Button::D])
        out += (out.size() == 1) ? "D" : "+D";

    if(button[Button::Taunt])
        out += (out.size() == 1) ? "P" : "+P";

    return out;
}

bool Player::State::inMove(int move) {
    int current = moveIndex;

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

void Player::State::setMove(int move, bool loop) {

    if(moveIndex != move) {
        moveIndex = move;
        moveFrame = 0;

    }else if(loop) {
        Animation* anim = g::save.getAnimation(config->move[moveIndex]);

        if(moveFrame >= anim->getFrameCount())
            moveFrame = 0;
    }
}

bool Player::State::doneMove() {
    Animation* anim = g::save.getAnimation(config->move[moveIndex]);
    return (moveFrame >= anim->getFrameCount());
}

int Player::State::getKeyFrame() {
    Animation* anim = g::save.getAnimation(config->move[moveIndex]);

    int key = 0;
    int time = 0;

    for(int i = 0; i < anim->keyFrames.size(); i ++) {

        if(moveFrame >= time)
            key = i;

        time += anim->keyFrames[i].duration;
    }
    return key;
}

Frame Player::State::getFrame() {
    Animation* anim = g::save.getAnimation(config->move[moveIndex]);

    Frame frame;

    if(anim) {
        frame = anim->getFrame(moveFrame);

        // Correct directional attributes of the frame
        frame.impulse.x *= side;

        for(auto& box : frame.hitBoxes) {

            if(side == -1)
                box.x = -box.x - box.w;

            box.x += position.x;
            box.y += position.y;

            box.force.x *= side;            
        }

        for(auto& box : frame.hurtBoxes) {

            if(side == -1)
                box.x = -box.x - box.w;

            box.x += position.x;
            box.y += position.y;        
        }

        for(int i = 0; i < frame.pose.jointCount; i ++) {
            frame.pose.joints[i].x *= side;
            frame.pose.joints[i] += position;
        }
    }
    return frame;
}

Skeleton Player::State::getSkeleton() {
    return getFrame().pose;
}

vector<HitBox> Player::State::getHitBoxes() {
    return getFrame().hitBoxes;
}

vector<HurtBox> Player::State::getHurtBoxes() {
    return getFrame().hurtBoxes;   
}

vector<Clothing*> Player::getClothes() {
    vector<Clothing*> out;

    for(auto& it : config.clothes)
        out.push_back(g::save.getClothing(it));

    return out;
}

Vector2 Player::State::getSOCD() {
    Vector2 mov;

    if(button[Button::Right])
        mov.x += 1;

    if(button[Button::Left])
        mov.x -= 1;

    if(button[Button::Up])
        mov.y += 1;

    if(button[Button::Down])
        mov.y -= 1;

    return mov;
}