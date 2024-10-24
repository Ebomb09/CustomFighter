#include "player.h"
#include "move.h"
#include "video.h"
#include "input_interpreter.h"
#include "save.h"
#include "math.h"
#include "save.h"
#include "audio.h"

#include <fstream>
#include <cmath>
#include <json.hpp>
#include <thread>

using std::vector, std::string;

static vector<int> aiSuccessorFunction(vector<Player>& players, int gameIndex) {
    Player& self = players[gameIndex];

    int target = self.getTarget(players);

    // Check preconditions if this player can move
    if( self.state.stun > 0 || 
        self.state.health <= 0 || 
        self.state.hitStop != 0 || 
        target < 0)
        return {};

    int moveIndex = self.state.moveIndex;
    string moveName = self.config.moves[moveIndex];
    Animation* currentAnim = self.getAnimations()[moveIndex];
    int moveCategory = (currentAnim) ? currentAnim->category : -1;

    // Accumulate all moves that can be performed from current state
    vector<int> moves;
    moves.push_back(-1);

    for(int i = 0; i < Move::Total; i ++) {
        Animation* anim = self.getAnimations()[i];

        if(anim) {
            // Also check if the customFrom is set
            if(anim->from[moveCategory] || anim->customFrom == moveName)
                moves.push_back(i);   
        }
    }
    return moves;
}

static int aiCalculateHeuristic(vector<Player>& players, int gameIndex) {
    int target = players[gameIndex].getTarget(players);

    if(target >= 0) {
        Player& self = players[gameIndex];
        Player& enem = players[target];

        int loop = self.aiLevel * 100;

        float selfHP = (self.state.health / 100.f) * 100.f;
        float enemHP = ((100 - enem.state.health) / 100.f) * 100.f;
        float dist = ((CameraBounds.w - std::abs(self.state.position.x - enem.state.position.x)) / CameraBounds.w) * 100.f;

        // Divide loop time into half to determine heuristic for game plan
        if((self.state.counter % loop) < (loop / 2)) {

            float wSelfHP = 0.75f * (self.aiLevel / 10.f);
            float wEnemHP = 0.75f * ((10 - self.aiLevel) / 10.f);
            float wDist = 0.25f;

            return wSelfHP*selfHP + wEnemHP*enemHP + wDist*dist;

        }else {
            return 0.25f*dist + 0.75f*selfHP;
        }


    }
    return 100;
}

static void aiRunSimulation(vector<Player>& players, int gameIndex, int& score) {

    for(int i = 0; i < 20; i ++) {
        vector<Player> others = players;

        for(auto& ply : players)
            ply.advanceFrame(others);
    }

    score = aiCalculateHeuristic(players, gameIndex);
}

static int aiSolve(vector<Player>& players, int gameIndex) {

    vector<int> nodes = aiSuccessorFunction(players, gameIndex);
    int move_index = 0;
    int value = -100000;

    vector<std::thread>     tasks(nodes.size());
    vector<vector<Player>>  simulations(nodes.size());
    vector<int>             scores(nodes.size());

    for(int i = 0; i < nodes.size(); i ++) {

        // Create simulation, assume perfect world opponents don't attack back
        simulations[i] = players;
        simulations[i][gameIndex].state.aiMove = nodes[i];

        tasks[i] = std::thread(aiRunSimulation, std::ref(simulations[i]), gameIndex, std::ref(scores[i]));
    }

    // Evaluate the scores
    for(int i = 0; i < nodes.size(); i ++) {
        tasks[i].join();

        if(scores[i] > value) {
            value = scores[i];
            move_index = i;
        }   
    }

    // Return a valid move
    if(nodes.size() > 0)
        return nodes[move_index];   

    return -1;
}

void Player::draw(Renderer* renderer) {

    if(!renderer)
        renderer = &g::video;

    Skeleton pose = getSkeleton();

    // HitStop shake
    if(state.hitStop < 0) {

        for(int i = 0; i < pose.jointCount; i ++) 
            pose.joints[i].x += std::sin(state.hitStop) * 2;
    }

    pose.draw(renderer, getClothes(), state.look, config.armSize, config.legSize);
}

void Player::drawShadow(Renderer* renderer) {

    if(!renderer)
        renderer = &g::video;

    // Draw Shadow
    float height = 16.f;

    Rectangle bounds = getRealBoundingBox();
    bounds.x -= 4.f;
    bounds.w += 8.f;
    bounds.h = bounds.y;
    bounds = renderer->toScreen(bounds);

    sf::CircleShape shad;
    shad.setPosition({bounds.x, bounds.y + bounds.h - height / 2.f});
    shad.setRadius(bounds.w / 2.f);
    shad.setScale({1.f, height / bounds.w});
    shad.setFillColor(sf::Color(0, 0, 0, 128));
    renderer->draw(shad);
}

void Player::drawEffects(Renderer* renderer) {

    if(!renderer)
        renderer = &g::video;

    // Draw effects
    for(int i = 0; i < Effect::Max; i ++) {
        Effect& eff = state.effects[i];

        if(eff.id >= 0) {
            AnimatedTexture anim = g::save.getEffectByID(eff.id);
            sf::Texture* tex = anim.getFrame(eff.counter, eff.lifeTime);

            if(tex) {
                Rectangle rect {
                    eff.position.x,
                    eff.position.y,
                    eff.size.x,
                    eff.size.y
                };

                sf::RectangleShape d = renderer->toScreen(rect);
                d.setTexture(tex);
                d.setOrigin({d.getSize().x / 2, d.getSize().y / 2});
                d.setRotation(eff.angle * 180 / PI);
                renderer->draw(d);
            }
        }
    }
}

Button::Flag Player::readInput() {
    Button::Flag out;

    // Get the local save button config
    Button::Config buttonConfig = g::save.getButtonConfig(seatIndex);

    out.A       = g::input.held(buttonConfig.index, buttonConfig.A);
    out.B       = g::input.held(buttonConfig.index, buttonConfig.B);
    out.C       = g::input.held(buttonConfig.index, buttonConfig.C);
    out.D       = g::input.held(buttonConfig.index, buttonConfig.D);
    out.Taunt   = g::input.held(buttonConfig.index, buttonConfig.Taunt);
    out.Up      = g::input.held(buttonConfig.index, buttonConfig.Up);
    out.Down    = g::input.held(buttonConfig.index, buttonConfig.Down);
    out.Left    = g::input.held(buttonConfig.index, buttonConfig.Left);
    out.Right   = g::input.held(buttonConfig.index, buttonConfig.Right);

    return out;
}

Button::Flag Player::readInput(vector<Player>& others) {

    // Check if ai
    if(seatIndex == -1 && aiLevel > 0) {
        g::audio.disable();
        state.aiMove = aiSolve(others, gameIndex);
        g::audio.enable();
        return {};
    }
    return readInput();
}

void Player::setState(const Player::State& _state) {
    state = _state;

    // Reset the cache values to prevent issues rolling back
    bool enabled = cache.enabled;
    cache = Player::Cache();
    cache.enabled = enabled;
}

void Player::advanceFrame(vector<Player>& others) {

    // Hitstop
    if(state.hitStop > 0)
        state.hitStop --;

    if(state.hitStop < 0)
        state.hitStop ++;

    // Update the last hit key frames
    for(int i = 0; i < others.size(); i ++) {

        if(others[i].getKeyFrame() != state.hitKeyFrame[i])
            state.hitKeyFrame[i] = -1;
    }

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

    // Taunt cancel from anything
    if(getTaggedIn(others))
        if((in.Taunt || state.aiMove == Move::Taunt) && state.position.y <= 0 && getFrame().cancel)
            setMove(Move::Taunt);

    // Add input to the button history
    for(int i = Button::History - 1; i > 0; i --) 
        state.button[i] = state.button[i - 1];

    state.button[0] = in;

    // Special Move Check
    if(getFrame().cancel && getTaggedIn(others)) {
        int move = searchBestMove(getInputBuffer());

        if(move != -1) {
            state.moveFrame = 0;
            setMove(move);
        }
    }

    // Exit early, if in HitStop
    if(state.hitStop != 0) 
        return;

    // Increment frames
    state.moveFrame ++;
    state.counter ++;

    if(state.stun > 0)
        state.stun --;

    // Stop pushback after first frame of hitstop ended
    state.pushBack = {0, 0};

    // Exit early, if in the air and opponents are tagging
    if(inMove(Move::JumpCombo)) {
        bool tagging = false;
        int count = 0;

        for(auto& ply : others) {

            if(ply.gameIndex != gameIndex && ply.team != team & ply.state.health > 0) {
                count ++;

                if(ply.getTaggedIn(others) && ply.inMove(Move::Taunt))
                    tagging = true;
            }
        }

        if(count > 1 && tagging)
            return;
    }

    // Grabber state control
    if(getFrame().isGrab) {

        // Get current grabee and the previous
        int prevIndex = -1;
        state.grabeeIndex = -1;

        for(auto& ply : others) {

            if(ply.gameIndex == gameIndex)
                prevIndex = ply.state.grabeeIndex;

            if(ply.state.grabIndex == gameIndex) 
                state.grabeeIndex = ply.gameIndex;
        }

        // Was grab broken
        if(state.grabeeIndex < 0 && prevIndex >= 0){            
            state.stun = 20;
            state.velocity.x = state.side * -3.f;
            setMove(Move::StandCombo);

        // Was whiffed
        }else if(state.grabeeIndex < 0 && getFrame().cancel) {
            setMove(Move::Stand);
        }

    }else {
        state.grabeeIndex = -1;
    }

    // Grabee state control
    if(state.grabIndex >= 0) {

        // Force set to the previous frame
        for(auto& ply : others) {

            if(ply.gameIndex == gameIndex) {
                cache.frameCounter = state.counter;
                cache.frame = ply.cache.frame;
            }
        }

        // Check if the inputs match the required grab break
        bool inputBreak = false;

        switch(others[state.grabIndex].getFrame().grabBreak) {

            case GrabBreak::A:
                inputBreak = state.button[0].A;
                break;

            case GrabBreak::C:
                inputBreak = state.button[0].C;
                break;

            case GrabBreak::AC:
                inputBreak = state.button[0].A && state.button[0].C;
                break;
        }

        // Break if grabbed at the same time
        if(others[state.grabIndex].state.grabIndex == gameIndex)
            inputBreak = true;

        // Get animation from the grabber
        if(others[state.grabIndex].getFrame().isGrab && !inputBreak) {
            cache.frame = Frame();
            cache.frame.pose = others[state.grabIndex].getFrame().grabee;
            cache.frame.key = others[state.grabIndex].getFrame().key;
            cache.frame.duration = others[state.grabIndex].getFrame().duration;
            state.fromMoveIndex = -1;
            state.fromMoveFrame = -1;

            Rectangle bounds = getRealBoundingBox();

            state.position = {
                bounds.x + bounds.w / 2.f,
                bounds.y - bounds.h
            };

        // Broke out of the grab
        }else if(inputBreak){
            state.grabIndex = -1;
            state.position.y = 0;
            state.velocity.x = state.side * -3.f;
            state.stun = 20;
            setMove(Move::StandCombo);

        // Grabber is no longer in the grab state
        }else {
            state.grabIndex = -1;

            // Set recovery state
            if(state.position.y > 0) 
                setMove(Move::EndCombo);
            else
                setMove(Move::GetUp);

            // Dead from last hit, set to end of knockback
            if(state.health <= 0) {
                Animation* anim = getAnimations()[Move::KnockDown];

                if(anim) {
                    state.moveIndex = Move::KnockDown;
                    state.moveFrame = anim->getFrameCount();
                }
            }
        }
    }

    // Check for pushBack
    for(auto& ply : others) {

        if(ply.team != team)
            state.velocity += ply.state.pushBack;
    }

    // Check Hit/Hurtbox Collisions
    Collision col = getCollision(others);

    if(col.collided) {

        if(col.block) {

            if(col.hitBox.type != HitType::Grab) {
                state.velocity.x += col.hitBox.force.x;

                state.stun = col.hitBox.blockStun;
                state.accDamage = col.hitBox.blockStun;

                if(inMove(Move::Stand) || inMove(Move::StandBlock)){
                    setMove(Move::StandBlock);

                }else{
                    setMove(Move::CrouchBlock);
                }
            }

        }else{

            // Prevent launcher looping in the corner
            if(col.hitBox.force.y > 0 && state.position.y > 0 && inMove(Move::JumpCombo))
                col.hitBox.knockdown = true;

            // Float if a neutral hit, and already airborne
            if(col.hitBox.force.y == 0 && state.position.y > 0)
                col.hitBox.force.y = 2;
                
            state.velocity.x += col.hitBox.force.x;
            state.velocity.y = col.hitBox.force.y;

            dealDamage(col.hitBox.damage);

            // Signal grab
            if(col.hitBox.type == HitType::Grab) {
                state.grabIndex = col.index;

            // Signal tumble to reset via knockdown
            }else if(col.hitBox.knockdown) {
                setMove(Move::EndCombo);

            // Set airborne state
            }else if(state.position.y > 0 || state.velocity.y > 0){
                setMove(Move::JumpCombo, true);

            // Set crouching hit state
            }else if(inMove(Move::Crouch) || inMove(Move::CrouchBlock) || inMove(Move::CrouchCombo)){
                setMove(Move::CrouchCombo, true);
                state.stun = col.hitBox.hitStun;

            // Set standing hit state
            }else{
                setMove(Move::StandCombo);
                state.stun = col.hitBox.hitStun;
            }
        }

        // If cornered self pushback
        if(inCorner())
            state.pushBack.x = -state.velocity.x;

        // Set state to prevent being hit until the next keyframe
        state.hitKeyFrame[col.index] = others[col.index].getKeyFrame();
    }

    // Movement
    state.velocity += getFrame().impulse;
    state.position += state.velocity;

    // Collision checks
    if(getTarget(others) != -1) {

        // Unstick players
        while((state.position - others[getTarget(others)].state.position).getDistance() < 25) {

            if(state.position.x < others[getTarget(others)].state.position.x)
                state.position.x -= 1;

            else if(state.position.x == others[getTarget(others)].state.position.x)
                state.position.x += (state.position.x < 0) ? 1 : -1;

            else
                state.position.x += 1;
        }   
    }

    if(getTaggedIn(others)) {
        Vector2 center = getCameraCenter(others);

        // Clamp position within the camera
        state.position.x = std::clamp(
            state.position.x, 
            center.x - CameraBounds.w / 2 + 16,
            center.x + CameraBounds.w / 2 - 16
            );

        // Clamp position within stage bounds
        state.position.x = std::clamp(
            state.position.x, 
            StageLeft, 
            StageRight
            );  

        // Wait for teammate to take position before passing tag
        if(inMove(Move::Taunt) || state.health <= 0) {

            for(auto& ply : others) {

                if(ply.gameIndex != gameIndex && ply.team == team && ply.state.health > 0) {

                    if( (state.side > 0 && ply.state.position.x > state.position.x) ||
                        (state.side < 0 && ply.state.position.x < state.position.x)) {
                        state.tagCount ++;
                    }
                }
            }
        }            
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

        // If the move is an air move cancel early if on ground
        Animation* anim = getAnimations()[state.moveIndex];

        if(anim && (anim->category == MoveCategory::AirNormal || anim->category == MoveCategory::AirSpecial || anim->category == MoveCategory::AirCommandNormal)) {
            setMove(Move::Stand);
        }

        // Anticipate the jump startup and only return once animation is done
        if(inMove(Move::Jump) && doneMove())
            setMove(Move::Stand);

        // Taunt cancel
        if(inMove(Move::Taunt) && doneMove())
            setMove(Move::Stand);

        // Player stay down once defeated
        if(state.health <= 0) {
            setMove(Move::KnockDown);

            // Signal tagCount is to be ignored when there is still more alive team members
            if(doneMove()) {

                for(auto& ply : others) {
                    if(ply.team == team && ply.state.health > 0){
                        state.tagCount = -1;
                        break;
                    }
                }
            }
        }

        // Hit States
        if(inMove(Move::JumpCombo) || inMove(Move::EndCombo)) 
            setMove(Move::KnockDown);

        if(inMove(Move::KnockDown) && doneMove() && state.health > 0)
            setMove(Move::GetUp);

        if((inMove(Move::StandCombo) || inMove(Move::StandBlock)) && state.stun == 0) 
            setMove(Move::Stand);

        if((inMove(Move::CrouchCombo) || inMove(Move::CrouchBlock)) && state.stun == 0) 
            setMove(Move::Crouch);

        if(inMove(Move::GetUp) && doneMove())
            setMove(Move::Stand);

        // Controls
        if(getTaggedIn(others)) {

            // Cancel the special tag motions
            if((inMove(Move::TagIn) || inMove(Move::TagOut)))
                setMove(Move::Stand);

            // Movement options
            if(inMove(Move::Stand) || inMove(Move::Crouch)) {

                // If on ground look in direction of opponent
                if(getTarget(others) != -1)
                    state.side = (state.position.x < others[getTarget(others)].state.position.x) ? 1 : -1;
        
                // SOCD movement
                if((getSOCD().y == 0 && getSOCD().x == state.side) || state.aiMove == Move::WalkForwards) {
                    setMove(Move::WalkForwards, true);

                }else if((getSOCD().y == 0 && getSOCD().x == -state.side) || state.aiMove == Move::WalkBackwards) {
                    setMove(Move::WalkBackwards, true);

                }else if((getSOCD().y == 0 && getSOCD().x == 0) || state.aiMove == Move::Stand) {
                    setMove(Move::Stand, true);

                }else if((getSOCD().y > 0 && getSOCD().x == state.side) || state.aiMove == Move::JumpForwards) {
                    setMove(Move::JumpForwards);

                }else if((getSOCD().y > 0 && getSOCD().x == -state.side) || state.aiMove == Move::JumpBackwards) {
                    setMove(Move::JumpBackwards);

                }else if((getSOCD().y > 0 && getSOCD().x == 0) || state.aiMove == Move::Jump) {
                    setMove(Move::Jump);

                }else if(getSOCD().y < 0 || state.aiMove == Move::Crouch) {
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

                if(ply.team == team && ply.gameIndex != gameIndex && ply.getTaggedIn(others)) {
                    curr = i;
                    prepare = ply.inMove(Move::Taunt) || ply.state.health <= 0;
                }
            }

            Vector2 center = getCameraCenter(others);

            // Move in
            if(prepare) {

                // Clamp within the camera once ready
                state.position.x = std::clamp(state.position.x, center.x - CameraBounds.w / 2 - 16, center.x + CameraBounds.w / 2 + 16);
                setMove(Move::TagIn, true);

            // In camera area, tag out
            }else if(state.position.x > center.x - CameraBounds.w / 2 - 16 && state.position.x < center.x + CameraBounds.w / 2 + 16) {

                if(inMove(Move::Stand))
                    setMove(Move::TagOut, true);

            // Not in camera area, side line and prepare to move in
            }else if(curr != -1) {
                state.side = others[curr].state.side;

                // Just off screen
                state.position = {
                    (state.side > 0) ? StageBounds.x - 64 : StageBounds.x + StageBounds.w + 64,
                    0
                };
                state.velocity = {0, 0};
            }
        }

    }else{
        state.velocity.y -= 0.25;

        // Loop the ender 
        if(inMove(Move::EndCombo))
            setMove(Move::EndCombo, true);
    }

    // Look at the target while still alive
    if(state.health > 0) {

        if(getTarget(others) != -1 && state.stun == 0 && state.grabIndex < 0) {
            Skeleton myPose = getFrame().pose;
            Skeleton opPose = others[getTarget(others)].getSkeleton();

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

    // Move Index changed, interpolate signal interpolation
    for(auto& ply : others) {

        if(ply.gameIndex == gameIndex) {

            if(ply.state.moveIndex != state.moveIndex && ply.state.grabIndex < 0) {
                state.fromMoveIndex = ply.state.moveIndex;
                state.fromMoveFrame = ply.state.moveFrame;
            }
            break;
        }
    }

    // Reset input
    in = Button::Flag();
    state.aiMove = -1;
}

void Player::advanceEffects(std::vector<Player>& others) {

    // Step effect timers
    for(int i = 0; i < Effect::Max; i ++) {

        if(state.effects[i].id >= 0) {
            state.effects[i].counter ++;

            // Once complete reset to default
            if(state.effects[i].counter > state.effects[i].lifeTime) 
                state.effects[i] = Effect();
        }
    }

    // State frames progressed, continue showing effects
    if(others[gameIndex].state.counter == state.counter)
        return;

    // Play sound effect related to frame of animation
    if(getFrame().sound != "")
        g::audio.playSound(g::save.getSound(getFrame().sound), true);

    // Check collisions
    Collision col = getCollision(others);

    if(col.collided) {

        if(col.block) {
            g::audio.playSound(g::save.getSound("block"), true);

        }else {

            if(col.hitBox.damage <= 10) {
                g::audio.playSound(g::save.getSound("hit_light"), true);      

            }else {
                g::audio.playSound(g::save.getSound("hit_hard"), true);      
            }
        }

        // Create hitspark effect
        for(int i = 0; i < Effect::Max; i ++) {
            Effect& eff = state.effects[i];

            if(eff.id == -1) {

                if(col.block) {
                    eff.id = g::save.getEffect("blockspark").id;
                    eff.lifeTime = std::max(24, col.hitBox.damage * 2);

                }else {
                    eff.id = g::save.getEffect("hitspark").id;
                    eff.lifeTime = std::max(12, col.hitBox.damage);
                }
                eff.position = col.location;
                eff.angle = (Vector2(col.hitBox.x + col.hitBox.w / 2, col.hitBox.y - col.hitBox.h / 2) - col.location).getAngle();
                eff.size = {24, 24};
                break;
            }
        }
    }
}

void Player::dealDamage(int dmg) {
    state.health -= dmg;
    state.accDamage += dmg;

    if(state.health < 0)
        state.health = 0;
}

const int& Player::getTarget(vector<Player>& others) {

    if(cache.enabled && cache.targetCounter == state.counter)
        return cache.target;

    cache.target = -1;
    cache.targetCounter = state.counter;

    if(!getTaggedIn(others)) 
        return cache.target;

    for(int i = 0; i < others.size(); i ++) {

        if(others[i].team != team && others[i].getTaggedIn(others)) {
            cache.target = i;
            break;
        }
    }
    return cache.target;
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

bool Player::doneMove() {
    Animation* anim = getAnimations()[state.moveIndex];

    if(!anim)
        return true;

    return (state.moveFrame >= anim->getFrameCount());
}

void Player::setMove(int move, bool loop) {
    Animation* anim = getAnimations()[move];

    if(!anim)
        return;

    // Changing to a new move
    if(state.moveIndex != move) {
        state.moveIndex = move;
        state.moveFrame = 0;

    }else if(loop) {

        if(state.moveFrame >= anim->getFrameCount())
            state.moveFrame = 0;
    }
}

vector<string> Player::getInputBuffer() {
    vector<string> buffer;

    for(int i = 0; i < Button::History-1; i ++) {

        // Directional presses
        string motion = "";
        Vector2 socd;

        bool motionChange = state.button[i].Up != state.button[i+1].Up ||
            state.button[i].Down != state.button[i+1].Down ||
            state.button[i].Left != state.button[i+1].Left ||
            state.button[i].Right != state.button[i+1].Right ||
            state.button[i].A && !state.button[i+1].A ||
            state.button[i].B && !state.button[i+1].B ||
            state.button[i].C && !state.button[i+1].C ||
            state.button[i].D && !state.button[i+1].D ||
            state.button[i].Taunt && !state.button[i+1].Taunt;

        if(motionChange && state.button[i].Up)          socd.y += 1;
        if(motionChange && state.button[i].Down)        socd.y -= 1;
        if(motionChange && state.button[i].Right)       socd.x += 1 * state.side;
        if(motionChange && state.button[i].Left)        socd.x -= 1 * state.side;

        motion = ('5' + (int)socd.x + (int)socd.y * 3);

        // Button presses
        bool buttonChange = state.button[i].A && !state.button[i+1].A ||
            state.button[i].B && !state.button[i+1].B ||
            state.button[i].C && !state.button[i+1].C ||
            state.button[i].D && !state.button[i+1].D ||
            state.button[i].Taunt && !state.button[i+1].Taunt;

        string button = "";
        if(buttonChange && state.button[i].A)       button += (button.size() == 0) ? "A" : "+A";
        if(buttonChange && state.button[i].B)       button += (button.size() == 0) ? "B" : "+B";
        if(buttonChange && state.button[i].C)       button += (button.size() == 0) ? "C" : "+C";
        if(buttonChange && state.button[i].D)       button += (button.size() == 0) ? "D" : "+D";
        if(buttonChange && state.button[i].Taunt)   button += (button.size() == 0) ? "P" : "+P";

        buffer.push_back(motion + button);
    }
    return buffer;
}

vector<string> Player::getMotionBuffer(const string& motion) {
    vector<string> buffer;
    bool lastInput = true;

    for(int i = motion.size()-1; i >= 0; i --) {

        if(lastInput && motion[i] != 'A' && motion[i] != 'B' && motion[i] != 'C' && motion[i] != 'D' && motion[i] != '+') {
            buffer.push_back(motion.substr(i));
            lastInput = false;

        }else if(!lastInput) {
            buffer.push_back(motion.substr(i, 1));
        }
    }

    if(lastInput)
        buffer.push_back(motion);

    return buffer;
}

bool Player::matchLeftConform(const std::string& a, const std::string& b) {
    int u = 0;
    int v = 0;

    bool hasButton[2] {false, false};

    for(auto& ch : a) {

        if(ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' || ch == 'P') {
            hasButton[0] = true;
            break;
        }
    }

    for(auto& ch : b) {

        if(ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' || ch == 'P') {
            hasButton[1] = true;
            break;
        }
    }

    // a and b should both either contain a button or not contain a button
    if(hasButton[0] != hasButton[1])
        return false;

    while(u < a.size() && v < b.size()) {

        if(a[u] == b[v]) {
            u ++;
            v ++;

        }else {
            u ++;
        }

        if(v >= b.size())
            return true;
    }
    return false;
}

int Player::searchBestMove(const vector<string>& inputBuffer) {
    int best = -1;

    for(int i = Move::Custom00; i < Move::Total; i ++) {

        // Validate both animations
        Animation* currAnim = getAnimations()[state.moveIndex];
        Animation* nextAnim = getAnimations()[i];

        if(!currAnim || !nextAnim)
            continue;

        // Check valid to cancel into the next anim
        if(!nextAnim->from[currAnim->category] && nextAnim->customFrom != currAnim->name) 
            continue;

        // Get the buffer
        vector<string> motionBuffer = getMotionBuffer(config.motions[i]);

        // Scan the motion strings for matching inputs
        bool match = false;

        // Check if the player has an ai controlling moves
        if(state.aiMove == i) {
            match = true;

        }else {

            // Check last input matches before checking
            if(matchLeftConform(inputBuffer[0], motionBuffer[0])) {
                int u = 1;
                int v = 1;
                int consume = 0;

                while(!match) {

                    if(u >= inputBuffer.size()) {
                        break;
                    }

                    if(v >= motionBuffer.size()) {
                        match = true;
                        break;
                    }

                    if(matchLeftConform(inputBuffer[u], motionBuffer[v])) {
                        u ++;
                        v ++;
                        consume = 0;

                    }else {
                        u ++;
                        consume ++;
                    }

                    // Reached input threshold before invalid
                    if(consume >= 10)
                        break;
                }
            }
        }

        // A correctly matching move was found
        if(match) {

            if(best == -1)
                best = i;

            else if(config.motions[i].size() >= config.motions[best].size()) 
                best = i;
        }
    }
    return best;
}

bool Player::inCorner() {
    return (state.position.x <= StageLeft || state.position.x >= StageRight);
}

const bool& Player::getTaggedIn(vector<Player>& others) {

    if(cache.enabled && cache.taggedCounter == state.counter)
        return cache.tagged;

    cache.taggedCounter = state.counter;

    // Indicates out of the game
    if(state.tagCount < 0) {
        cache.tagged = false;
        return cache.tagged;
    }

    cache.tagged = true;

    for(auto& ply : others) {

        if(ply.gameIndex == gameIndex || ply.state.tagCount < 0)
            continue;

        if(ply.team == team) {

            if(ply.state.tagCount == state.tagCount) {
                cache.tagged = gameIndex < ply.gameIndex;
            
            }else {
                cache.tagged = (state.tagCount < ply.state.tagCount);
            }
            break;
        }
    }
    return cache.tagged;
}

const Player::Collision& Player::getCollision(vector<Player>& others) {

    if(cache.enabled && cache.collisionCounter == state.counter)
        return cache.collision;

    if(!getTaggedIn(others))
        return cache.collision;

    cache.collision = Collision();
    cache.collisionCounter = state.counter;

    for(int i = 0; i < others.size(); i ++) {

        if(others[i].team == team)
            continue;

        // Valid collision
        if(state.hitKeyFrame[i] == -1) {

            // Check HurtBox <-> HitBox collisions, for only one match
            for(auto& hit : others[i].getHitBoxes()) {

                if(hit.type == HitType::Grab && state.grabIndex >= 0) {
                    cache.collision.collided = true;
                    cache.collision.hitBox = hit;
                    cache.collision.index = i;
                    cache.collision.location = {
                        hit.x + hit.w/2,
                        hit.y - hit.h/2
                    };
                    break;
                }

                for(auto& hurt : getHurtBoxes()) {

                    if(Real::rectangleInRectangle(hurt, hit)) {
                        cache.collision.collided = true;
                        cache.collision.hitBox = hit;
                        cache.collision.index = i;
                        cache.collision.location = {
                            ((hit.x + hit.w/2) + (hurt.x + hurt.w/2))/2.f,
                            ((hit.y - hit.h/2) + (hurt.y - hurt.h/2))/2.f
                        };
                        break;
                    }
                }

                if(cache.collision.collided)
                    break;
            }
        }
    }

    // Check for block status
    if(cache.collision.collided) {
        bool block = false;

        // Block: if input in the opposite direction of opponent   
        if(inMove(Move::Stand) || inMove(Move::StandBlock) || inMove(Move::Crouch) || inMove(Move::CrouchBlock)) 
            block = (state.side == 1 && getSOCD().x < 0) || (state.side == -1 && getSOCD().x > 0);
        
        // Block: if standing or crouching input for hitType
        switch(cache.collision.hitBox.type) {

            case HitType::High: 
                cache.collision.block = (block && getSOCD().y == 0) || state.aiMove == Move::StandBlock;
                break;

            case HitType::Mid:
                cache.collision.block = (block) || state.aiMove == Move::CrouchBlock || state.aiMove == Move::StandBlock;
                break;

            case HitType::Low: 
                cache.collision.block = (block && getSOCD().y < 0) || state.aiMove == Move::CrouchBlock;
                break;

            case HitType::Unblockable:
                cache.collision.block = false;
                break;

            case HitType::Grab:
                cache.collision.block = (state.position.y > 0 || state.stun != 0) && state.grabIndex < 0;
                break;
        }
    }
    return cache.collision;
}

Vector2 Player::getCameraCenter(std::vector<Player>& others) {
    Vector2 pos;
    int n = 0;

    for(int i = 0; i < others.size(); i ++) {

        if(others[i].getTaggedIn(others)) {
            pos += others[i].state.position;
            n ++;
        }
    }
    return pos / n;
}

const int& Player::getKeyFrame() {
    return getFrame().key;
}

const Frame& Player::getFrame() {

    if(cache.enabled && cache.frameCounter == state.counter)
        return cache.frame;

    Animation* anim = getAnimations()[state.moveIndex];

    if(!anim)
        return cache.frame;

    // Attempt to use interpolated signals
    bool usingInterp = false;

    if(anim->getKeyFrame(0).duration > 1 && state.fromMoveFrame != -1 && state.fromMoveIndex != -1) {
        Animation* fromAnim = getAnimations()[state.fromMoveIndex];

        if(fromAnim) {
            Animation interp = *anim;

            // Calculate the maximum transition frames
            int diff = std::min(interp.getKeyFrame(0).duration / 2, 4);

            // Shorten the initial frame
            interp.getKeyFrame(0).duration -= diff;

            // Add the interpolated new frames
            Frame copy = interp.getKeyFrame(0);
            copy.duration = diff;
            copy.sound = "";
            copy.pose = fromAnim->getFrame(state.fromMoveFrame).pose;
            interp.insertKeyFrame(0, copy);

            cache.frame = interp.getFrame(state.moveFrame);

            // Manually fix the key frame index
            cache.frame.key = std::max(0, cache.frame.key - 1);

            usingInterp = true;
        }
    }

    // No interpolation
    if(!usingInterp)
        cache.frame = anim->getFrame(state.moveFrame);

    // No longer need to signal from previous moveIndex/Frame
    if(cache.frame.key > 0) {
        state.fromMoveIndex = -1;
        state.fromMoveFrame = -1;
    }

    cache.frameCounter = state.counter;

    // Correct directional attributes of the frame
    cache.frame.impulse.x *= state.side;

    for(auto& box : cache.frame.hitBoxes) {

        if(state.side == -1)
            box.x = -box.x - box.w;

        box.x += state.position.x;
        box.y += state.position.y;

        box.force.x *= state.side;            
    }

    for(auto& box : cache.frame.hurtBoxes) {

        if(state.side == -1)
            box.x = -box.x - box.w;

        box.x += state.position.x;
        box.y += state.position.y;        
    }

    for(int i = 0; i < cache.frame.pose.jointCount; i ++) {
        cache.frame.pose.joints[i].x *= state.side;
        cache.frame.pose.joints[i] += state.position;
    }

    for(int i = 0; i < cache.frame.grabee.jointCount; i ++) {
        cache.frame.grabee.joints[i].x *= state.side;
        cache.frame.grabee.joints[i] += state.position;
    }

    // Flip draw order array
    if(state.side == -1) {
        for(int i = 0; i < SkeletonDrawOrder::Total / 2; i ++) 
            std::swap(cache.frame.pose.order[i], cache.frame.pose.order[SkeletonDrawOrder::Total - 1 - i]);
    }
    return cache.frame;
}

const Skeleton& Player::getSkeleton() {
    return getFrame().pose;
}

const vector<HitBox>& Player::getHitBoxes() {
    return getFrame().hitBoxes;
}

const vector<HurtBox>& Player::getHurtBoxes() {
    return getFrame().hurtBoxes;
}

const vector<Clothing>& Player::getClothes() {

    if(cache.enabled && cache.clothes.size() > 0)
        return cache.clothes;

    vector<Clothing>& out = cache.clothes;
    out.clear();

    // Implied clothing... skin
    Clothing* skin = g::save.getClothing("skin");
    if(skin) out.push_back(*skin);

    for(int i = 0; i < config.clothes.size(); i ++) {
        Clothing* cloth = g::save.getClothing(config.clothes[i].name);

        if(cloth) {
            Clothing copy = *cloth;
            copy.blend = sf::Color(config.clothes[i].r, config.clothes[i].g, config.clothes[i].b);
            out.push_back(copy);            
        }
    }
    return out;
}

const vector<Animation*>& Player::getAnimations() {

    if(cache.enabled && cache.anims.size() > 0)
        return cache.anims;    

    vector<Animation*>& out = cache.anims;
    out.clear();

    for(int i = 0; i < Move::Total; i ++) 
        out.push_back(g::save.getAnimation(config.moves[i]));
    
    return out;
}

const Vector2& Player::getSOCD(int index) {

    if(cache.enabled && cache.socdCounter == state.counter)
        return cache.socd;

    Vector2 mov;

    if(state.button[index].Right)
        mov.x += 1;

    if(state.button[index].Left)
        mov.x -= 1;

    if(state.button[index].Up)
        mov.y += 1;

    if(state.button[index].Down)
        mov.y -= 1;

    cache.socd = mov;
    cache.socdCounter = state.counter;

    return cache.socd;
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

        for(auto& cloth : json["clothes"]) {
            Cloth add;

            if(cloth.is_object()) {

                if(cloth["name"].is_string()) {
                    add.name = cloth["name"];

                    if(cloth["r"].is_number_integer())
                        add.r = cloth["r"];

                    if(cloth["g"].is_number_integer())
                        add.g = cloth["g"];

                    if(cloth["b"].is_number_integer())
                        add.b = cloth["b"];                    
                }
                clothes.push_back(add);                
            }
        }
    }

    if(json["armSize"].is_number())
        armSize = json["armSize"];

    if(json["legSize"].is_number())
        armSize = json["legSize"];

    for(int i = 0; i < json["moves"].size(); i ++) 
        if(json["moves"][i].is_string())
            moves[i] = json["moves"][i];

    for(int i = 0; i < json["motions"].size(); i ++) 
        if(json["motions"][i].is_string())
            motions[i] = json["motions"][i];        
}

string Player::Config::saveToText() {
    nlohmann::json json;

    for(int i = 0; i < clothes.size(); i ++) {
        nlohmann::json cloth;
        cloth["name"] = clothes[i].name;
        cloth["r"] = clothes[i].r;
        cloth["g"] = clothes[i].g;
        cloth["b"] = clothes[i].b;
        json["clothes"][i] = cloth;            
    }

    json["armSize"] = armSize;
    json["legSize"] = legSize;

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
    Vector2 min, max;
    
    min = {1000, 1000};
    max = {-1000, -1000};

    for(int i = 0; i < pose.jointCount; i ++) {
        min = {std::min(min.x, pose.joints[i].x), std::min(min.y, pose.joints[i].y)};
        max = {std::max(max.x, pose.joints[i].x), std::max(max.y, pose.joints[i].y)};
    }

    Rectangle rect {min.x, max.y, max.x - min.x, max.y};

    // Ensure it encapsulates some part of the skeleton
    if(rect.w < 1.f) {
        float diff = 1.f - rect.w;
        rect.x -= diff / 2.f;
        rect.w = 1.f;
    }

    if(rect.h < 1.f) {
        float diff = 1.f - rect.w;
        rect.y += diff / 2.f;
        rect.h = 1.f;
    }

    return rect;
}

int Player::Config::calculatePoints() {
    int points = 0;

    for(int i = Move::Custom00; i < Move::Total; i ++) {
        Animation* anim = g::save.getAnimation(moves[i]);

        if(anim) {
            int base = 0;

            switch(anim->category) {

            case MoveCategory::Normal:
            case MoveCategory::AirNormal:
            case MoveCategory::Grab:         
            case MoveCategory::AirGrab:  
                base = 1;
                break;

            case MoveCategory::CommandNormal:
            case MoveCategory::AirCommandNormal:
                base = 2;
                break;

            case MoveCategory::Special:
            case MoveCategory::AirSpecial:
                base = 3;
                break;

            case MoveCategory::Super:
            case MoveCategory::AirSuper:
                base = 4;
                break;
            }

            if(base > 0)
                points += std::round(base / (1.f + std::floor(motions[i].size() / 4.f)));
        }
    }
    return points;
}