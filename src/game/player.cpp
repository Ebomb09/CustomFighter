#include <iostream>

#include "core/input_interpreter.h"
#include "core/math.h"
#include "core/save.h"

#include "player.h"

Player::Player() {
    State& state = getState(0);
    state.health = 100;
    state.stun = 0;
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

    pose.draw(getClothes(), look);
}

void Player::readInput(int gameFrame) {
    State& state = getState(gameFrame);

    // Get inputs via config and interpreter
    for(int i = Button::Up; i <= Button::Right; i ++) 
        state.button[i] = g::input.keyHeld[config.button[i]];

    for(int i = Button::A; i <= Button::Taunt; i ++)
        state.button[i] = g::input.keyPressed[config.button[i]];
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
                        state.velocity.x = hit.impulse.x * opState.side;

                        if(state.inMove(Move::Stand) || state.inMove(Move::StandBlock)){
                            state.setMove(Move::StandBlock);
                            state.stun = hit.blockStun;
                        
                        }else{
                            state.setMove(Move::CrouchBlock);
                            state.stun = hit.blockStun;
                        }

                    }else{
                        state.health -= hit.damage;
                        state.velocity = hit.impulse * Vector2(opState.side, 1);

                        if(hit.knockdown) {
                            state.setMove(Move::KnockDown);

                        }else if(state.position.y > 0){
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
        for(int i = 0; i < 10; i ++) {

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

    if(state.inMove(Move::Jump) && state.position.y <= 0) {
        state.setMove(Move::Stand);
    }

    if(state.inMove(Move::Stand) || state.inMove(Move::Crouch)) {

        // If on ground look in direction of opponent
        state.side = (state.position.x < opState.position.x) ? 1 : -1;

        if(state.getSOCD().y == 0) {
            state.setMove(Move::Stand, true);
            state.velocity.x = state.getSOCD().x * 2;

        }else if(state.getSOCD().y > 0) {
            state.setMove(Move::Jump);
            state.velocity.y = 6;
            state.velocity.x = state.getSOCD().x * 2;

        }else if(state.getSOCD().y < 0) {
            state.setMove(Move::Crouch, true);
        }      
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
        out += 'A';
    if(button[Button::B])
        out += 'B';
    if(button[Button::C])
        out += 'C';
    if(button[Button::D])
        out += 'D';
    if(button[Button::Taunt])
        out += 'P';

    return out;
}

bool Player::State::inMove(int move) {
    return moveIndex == move;
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

    if(anim)
        return anim->getFrame(moveFrame);
    else
        return Frame();
}

Skeleton Player::State::getSkeleton() {
    Frame frame = getFrame();
    Skeleton pose = frame.pose;

    for(int i = 0; i < pose.jointCount; i ++) {
        pose.joints[i].x *= side;
        pose.joints[i] += position;
    }
    return pose;
}

vector<HitBox> Player::State::getHitBoxes() {
    vector<HitBox> boxes;

    for(HitBox box : getFrame().hitBoxes) {

        if(side == -1) 
            box.x = -box.x - box.w;

        box.x += position.x;
        box.y += position.y;

        boxes.push_back(box);
    }
    return boxes;
}

vector<HurtBox> Player::State::getHurtBoxes() {
    vector<HurtBox> boxes;

    for(HurtBox box : getFrame().hurtBoxes) {

        if(side == -1) 
            box.x = -box.x - box.w;

        box.x += position.x;
        box.y += position.y;

        boxes.push_back(box);
    }
    return boxes;   
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