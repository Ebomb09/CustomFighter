#include "animation.h"
#include "move.h"

#include <fstream>
#include <filesystem>
#include <json.hpp>

Frame::Frame() : pose(), duration(1), impulse(), cancel(false) {}

Frame::Frame(Skeleton _pose, int _duration) : pose(_pose), duration(_duration), impulse(), cancel(false) {}

Frame::Frame(const Frame& k) { 
    *this = k; 
}    

Frame::Frame(Frame&& k) { 
    *this = k; 
}

Frame& Frame::operator=(const Frame& copy) { 
    pose        = copy.pose; 
    duration    = copy.duration;
    hitBoxes    = copy.hitBoxes;
    hurtBoxes   = copy.hurtBoxes;
    impulse     = copy.impulse;
    cancel      = copy.cancel;
    return *this; 
}

Frame& Frame::operator=(Frame&& move) { 
    pose        = std::move(move.pose); 
    duration    = std::move(move.duration);
    hitBoxes    = std::move(move.hitBoxes);
    hurtBoxes   = std::move(move.hurtBoxes);   
    impulse     = std::move(move.impulse); 
    cancel      = std::move(move.cancel);
    return *this; 
}

Animation::Animation() {
    name = "";
    category = 0;

    for(int i = 0; i < MoveCategory::Total; i ++)
        from[i] = false;

    customFrom = "";
}

Animation::Animation(const Animation& k) {
    *this = k;
} 

Animation::Animation(Animation&& k) {
    *this = k;
}

Animation& Animation::operator=(const Animation& copy) {
    name = copy.name;
    category = copy.category;

    for(int i = 0; i < MoveCategory::Total; i ++)
        from[i] = copy.from[i];

    customFrom = copy.customFrom;

    keyFrames = copy.keyFrames;
    return *this;    
}

Animation& Animation::operator=(Animation&& move) {
    name = std::move(move.name);
    category = std::move(move.category);

    for(int i = 0; i < MoveCategory::Total; i ++)
        from[i] = std::move(move.from[i]);

    customFrom = std::move(move.customFrom);

    keyFrames = std::move(move.keyFrames);
    return *this;
}

void Animation::swapKeyFrame(int a, int b) {
    std::swap(keyFrames[a], keyFrames[b]);
}

void Animation::insertKeyFrame(int index, Frame k) {
    keyFrames.insert(keyFrames.begin() + index, k);
}

void Animation::removeKeyFrame(int index) {
    keyFrames.erase(keyFrames.begin() + index);
}

Frame& Animation::getKeyFrame(int index) {
    return keyFrames[index];
}

int Animation::getKeyFrameCount() {
    return keyFrames.size();
}

Frame Animation::getFrame(float t) {
    Frame frame;
    float f_pos = 0;

    // Clamp frame value
    if(t < 0)
        t = 0;
    else if(t >= getFrameCount()-1)
        t = getFrameCount()-1;

    for(int i = 0; i < getKeyFrameCount(); i ++) {

        // Within KeyFrame timing
        if(t >= f_pos && t < f_pos + keyFrames[i].duration) {
            frame = keyFrames[i];

            // If there's a next frame interpolate
            if(i + 1 < getKeyFrameCount()) {
                Skeleton poseA = keyFrames[i].pose;                
                Skeleton poseB = keyFrames[i + 1].pose;

                float progress = (t - f_pos) / (float)keyFrames[i].duration;

                // Calculate (final - initial) and times by frame progression
                for(int j = 0; j < poseB.jointCount; j ++) {
                    Vector2 mov = (poseB.joints[j] - poseA.joints[j]) * progress;
                    frame.pose.joints[j] += mov;
                }
            }
            break;
        }

        f_pos += keyFrames[i].duration;
    }
    return frame;
}

int Animation::getFrameCount() {
    int total = 0;

    for(int i = 0; i < getKeyFrameCount(); i ++)
        total += keyFrames[i].duration;
    return total;
}

void Animation::saveToFile(std::string fileName) {
    std::fstream file(fileName, std::fstream::out | std::fstream::trunc);

    if(!file.good()) {
        file.close();
        return;
    }

    nlohmann::json json;

    json["category"] = MoveCategory::String[category];

    for(int i = 0; i < MoveCategory::Total; i ++) 
        json["from" + MoveCategory::String[i]] = from[i];
    
    json["customFrom"] = customFrom;

    // Save KeyFrames
    json["keyFrames"] = nlohmann::json::array();

    for(int i = 0; i < keyFrames.size(); i ++) {
        nlohmann::json add;

        add["duration"] = keyFrames[i].duration;
        add["cancel"]   = keyFrames[i].cancel;

        add["impulse"]["x"] = keyFrames[i].impulse.x;
        add["impulse"]["y"] = keyFrames[i].impulse.y;

        // Save Joints
        add["joints"] = nlohmann::json::array();

        for(int j = 0; j < keyFrames[i].pose.jointCount; j ++) {
            nlohmann::json obj;
            obj["x"] = keyFrames[i].pose.joints[j].x;
            obj["y"] = keyFrames[i].pose.joints[j].y;      
            add["joints"].push_back(obj);      
        } 

        // Save draw order
        add["order"] = keyFrames[i].pose.order;

        // Save Hitboxes
        add["hitBoxes"] = nlohmann::json::array();   

        for(int j = 0; j < keyFrames[i].hitBoxes.size(); j ++) {
            nlohmann::json obj;
            obj["x"]           = keyFrames[i].hitBoxes[j].x;
            obj["y"]           = keyFrames[i].hitBoxes[j].y;
            obj["w"]           = keyFrames[i].hitBoxes[j].w;
            obj["h"]           = keyFrames[i].hitBoxes[j].h;
            obj["damage"]      = keyFrames[i].hitBoxes[j].damage;
            obj["hitStun"]     = keyFrames[i].hitBoxes[j].hitStun;
            obj["blockStun"]   = keyFrames[i].hitBoxes[j].blockStun;
            obj["knockdown"]   = keyFrames[i].hitBoxes[j].knockdown;
            obj["force"]["x"]  = keyFrames[i].hitBoxes[j].force.x;
            obj["force"]["y"]  = keyFrames[i].hitBoxes[j].force.y;
            add["hitBoxes"].push_back(obj);
        } 

        // Save Hurtboxes
        add["hurtBoxes"] = nlohmann::json::array();    

        for(int j = 0; j < keyFrames[i].hurtBoxes.size(); j ++) {
            nlohmann::json obj;
            obj["x"]       = keyFrames[i].hurtBoxes[j].x;
            obj["y"]       = keyFrames[i].hurtBoxes[j].y;
            obj["w"]       = keyFrames[i].hurtBoxes[j].w;
            obj["h"]       = keyFrames[i].hurtBoxes[j].h;
            obj["armour"]  = keyFrames[i].hurtBoxes[j].armour;
            add["hurtBoxes"].push_back(obj);
        } 

        // Add to keyframes
        json["keyFrames"].push_back(add);
    }
    file << json;
    file.close();
}

void Animation::loadFromFile(std::string fileName) {
    keyFrames.clear();

    std::fstream file(fileName, std::fstream::in);

    if(!file.good()) {
        file.close();
        return;
    }

    // Get the stem name
    std::filesystem::path path(fileName);
    name = path.stem().string();

    // Parse json
    nlohmann::json json;

    try {
        json = nlohmann::json::parse(file);

    }catch(nlohmann::json::exception e) {
        return;
    }

    // Load categories
    if(json["category"].is_string()) {

        for(int i = 0; i < MoveCategory::Total; i ++) {

            if(json["category"] == MoveCategory::String[i]) {
                category = i;
                break;
            }
        }        
    }

    for(int i = 0; i < MoveCategory::Total; i ++) {

        if(json["from" + MoveCategory::String[i]].is_boolean())
            from[i] = json.value("from" + MoveCategory::String[i], false);
    }

    if(json["customFrom"].is_string()) 
        customFrom = json.value("customFrom", "");

    for(auto& frame : json["keyFrames"]) {
        Frame newFrame;

        if(frame["duration"].is_number_integer())
            newFrame.duration = frame.value("duration", 0);

        if(frame["cancel"].is_boolean())
            newFrame.cancel = frame.value("cancel", false);

        if(frame["impulse"].is_object()) {

            if(frame["impulse"]["x"].is_number())
                newFrame.impulse.x = frame["impulse"].value("x", 0.f);

            if(frame["impulse"]["y"].is_number())            
                newFrame.impulse.y = frame["impulse"].value("y", 0.f);
        }

        int i = 0;
        for(auto& joint : frame["joints"]) {

            if(joint["x"].is_number_float())
                newFrame.pose.joints[i].x = joint.value("x", 0.f);

            if(joint["y"].is_number_float())
                newFrame.pose.joints[i].y = joint.value("y", 0.f);   

            i ++;
        }

        i = 0;
        for(auto& order : frame["order"]) {

            if(order.is_number_integer())
                newFrame.pose.order[i] = order;

            i ++;
        }

        for(auto& hitBox : frame["hitBoxes"]) {
            HitBox add;
            add.x           = hitBox.value("x", 0);
            add.y           = hitBox.value("y", 0);
            add.w           = hitBox.value("w", 0);
            add.h           = hitBox.value("h", 0);  
            add.damage      = hitBox.value("damage", 0);
            add.hitStun     = hitBox.value("hitStun", 0);
            add.blockStun   = hitBox.value("blockStun", 0);
            add.knockdown   = hitBox.value("knockdown", false);

            if(hitBox["force"].is_object()) {

                if(hitBox["force"]["x"].is_number_float())
                    add.force.x = hitBox["force"].value("x", 0.f);  

                if(hitBox["force"]["y"].is_number_float())
                    add.force.y = hitBox["force"].value("y", 0.f);            
            }
            
            newFrame.hitBoxes.push_back(add);
        }

        for(auto& hurtBox : frame["hurtBoxes"]) {
            HurtBox add;
            add.x       = hurtBox.value("x", 0);
            add.y       = hurtBox.value("y", 0);
            add.w       = hurtBox.value("w", 0);
            add.h       = hurtBox.value("h", 0); 
            add.armour  = hurtBox.value("armour", 0);

            newFrame.hurtBoxes.push_back(add);
        }    
        keyFrames.push_back(newFrame);
    }
    file.close();
}