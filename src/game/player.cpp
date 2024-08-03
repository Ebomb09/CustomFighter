#include <cmath>

#include "core/render_instance.h"
#include "core/input_interpreter.h"
#include "core/math.h"
#include "core/save.h"

#include "player.h"

void Player::draw() {

    Skeleton pose = getSkeleton();
    Skeleton opPose = config.opponent->getSkeleton();

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

Button Player::readInput() {
    Button out;

    out.A           = g::input.keyPressed[config.button.A];
    out.B           = g::input.keyPressed[config.button.B];        
    out.C           = g::input.keyPressed[config.button.C];
    out.D           = g::input.keyPressed[config.button.D];   
    out.Taunt       = g::input.keyPressed[config.button.Taunt];   

    // Any attack button is pressed then retrigger all held attack buttons
    if(out.A || out.B || out.C || out.D || out.Taunt) {
        out.A           = g::input.keyHeld[config.button.A];
        out.B           = g::input.keyHeld[config.button.B];        
        out.C           = g::input.keyHeld[config.button.C];
        out.D           = g::input.keyHeld[config.button.D];   
        out.Taunt       = g::input.keyHeld[config.button.Taunt];          
    }

    // Movement keys always held down
    out.Up = g::input.keyHeld[config.button.Up];
    out.Down = g::input.keyHeld[config.button.Down];        
    out.Left = g::input.keyHeld[config.button.Left];
    out.Right = g::input.keyHeld[config.button.Right];    

    return out;
}

void Player::advanceFrame(Button in) {

    // Add input to the button history
    for(int i = Button::History - 1; i > 0; i --) {
        state.button[i] = state.button[i - 1];
    }
    state.button[0] = in;   

    // Shortcut to the config
    Player* op = config.opponent;

    // Hitstop
    if(state.hitStop > 0)
        state.hitStop --;

    if(state.hitStop < 0)
        state.hitStop ++;

    // Check if taken any damage and set the HitStop
    int dmg1 = state.accDamage;
    int dmg2 = op->state.accDamage;        

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
    if(op->getKeyFrame() != state.hitKeyFrame)
        state.hitKeyFrame = -1;

    bool damageValid = (state.hitKeyFrame != op->getKeyFrame());

    if(damageValid) {

        // Check HurtBox <-> HitBox collisions, for only one match
        bool found = false;

        for(auto& hurt : getHurtBoxes()) {
            for(auto& hit : op->getHitBoxes()) {

                if(Real::rectangleInRectangle(hurt, hit)) {

                    // Set what keyframe you were hit on
                    state.hitKeyFrame = op->getKeyFrame();        

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
            string str = config.motion[i];
            int u = 0;
            int v = 0;
            bool match = false;

            // Check if in the required stance
            Animation* anim = g::save.getAnimation(config.move[i]);

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

                else if(config.motion[i].size() > config.motion[best].size()) 
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
        state.side = (state.position.x < op->state.position.x) ? 1 : -1;

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
    if((state.position - op->state.position).getDistance() < 25) {
        int side = (state.position.x < op->state.position.x) ? 1 : -1;
        state.position.x = op->state.position.x + 25 * -side;
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
        Animation* anim = g::save.getAnimation(config.move[state.moveIndex]);

        if(state.moveFrame >= anim->getFrameCount())
            state.moveFrame = 0;
    }
}

bool Player::doneMove() {
    Animation* anim = g::save.getAnimation(config.move[state.moveIndex]);
    return (state.moveFrame >= anim->getFrameCount());
}

int Player::getKeyFrame() {
    Animation* anim = g::save.getAnimation(config.move[state.moveIndex]);

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
    Animation* anim = g::save.getAnimation(config.move[state.moveIndex]);

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