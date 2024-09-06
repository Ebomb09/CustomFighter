#include "animation.h"
#include "move.h"

#include <fstream>
#include <json.hpp>

Animation::Animation() {

    for(int i = 0; i < MoveCategory::Total; i ++) 
        from[i] = false;
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

Frame Animation::getFrame(int t) {
    Frame frame;
    int f_pos = 0;

    // Clamp frame value
    t = std::clamp(t, 0, getFrameCount()-1);

    for(int i = 0; i < getKeyFrameCount(); i ++) {

        // Within KeyFrame timing
        if(t >= f_pos && t < f_pos + keyFrames[i].duration) {
            frame = keyFrames[i];

            // Only play sound effect on first active frame
            if(t != f_pos)
                frame.sound = "";
            
            // Prevent cancelling first active hit frame
            if(t == f_pos && keyFrames[i].duration > 1 && keyFrames[i].hitBoxes.size() > 0)
                frame.cancel = false;

            // If there's a next frame interpolate
            if(i + 1 < getKeyFrameCount()) {
                Skeleton poseA = keyFrames[i].pose;                
                Skeleton poseB = keyFrames[i + 1].pose;

                // Calculate total remaining duration of frame
                frame.duration = keyFrames[i].duration - (t - f_pos);

                // Calculate (final - initial) and times by frame progression
                float progress = (t - f_pos) / (float)keyFrames[i].duration;

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

void Animation::saveToFile(std::filesystem::path path) {
    std::fstream file(path, std::fstream::out | std::fstream::trunc);

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
        nlohmann::json newFrame;

        // Key Frame Properties
        newFrame["duration"] = keyFrames[i].duration;
        newFrame["cancel"]   = keyFrames[i].cancel;
        newFrame["sound"]    = keyFrames[i].sound;

        newFrame["impulse"]["x"] = keyFrames[i].impulse.x;
        newFrame["impulse"]["y"] = keyFrames[i].impulse.y;

        // Save Joints
        newFrame["joints"] = nlohmann::json::array();

        for(int j = 0; j < keyFrames[i].pose.jointCount; j ++) {
            nlohmann::json obj;
            obj["x"] = keyFrames[i].pose.joints[j].x;
            obj["y"] = keyFrames[i].pose.joints[j].y;      
            newFrame["joints"].push_back(obj);      
        } 

        // Save draw order
        newFrame["order"] = keyFrames[i].pose.order;

        // Save Hitboxes
        newFrame["hitBoxes"] = nlohmann::json::array();   

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
            newFrame["hitBoxes"].push_back(obj);
        } 

        // Save Hurtboxes
        newFrame["hurtBoxes"] = nlohmann::json::array();    

        for(int j = 0; j < keyFrames[i].hurtBoxes.size(); j ++) {
            nlohmann::json obj;
            obj["x"]       = keyFrames[i].hurtBoxes[j].x;
            obj["y"]       = keyFrames[i].hurtBoxes[j].y;
            obj["w"]       = keyFrames[i].hurtBoxes[j].w;
            obj["h"]       = keyFrames[i].hurtBoxes[j].h;
            obj["armour"]  = keyFrames[i].hurtBoxes[j].armour;
            newFrame["hurtBoxes"].push_back(obj);
        } 

        // Add to keyframes
        json["keyFrames"].push_back(newFrame);
    }
    file << json;
    file.close();
}

void Animation::loadFromFile(std::filesystem::path path) {
    keyFrames.clear();

    std::fstream file(path, std::fstream::in);

    if(!file.good()) {
        file.close();
        return;
    }

    // Parse json
    nlohmann::json json;

    try {
        json = nlohmann::json::parse(file);

    }catch(nlohmann::json::exception e) {
        return;
    }

    // Reset to default object
    *this = Animation();

    // Get the stem name
    name = path.stem().string();

    // Load category
    if(json["category"].is_string()) {

        for(int i = 0; i < MoveCategory::Total; i ++) {

            if(json["category"] == MoveCategory::String[i]) {
                category = i;
                break;
            }
        }        
    }

    // Load the cancel properties
    for(int i = 0; i < MoveCategory::Total; i ++) {

        if(json["from" + MoveCategory::String[i]].is_boolean())
            from[i] = json["from" + MoveCategory::String[i]];
    }

    if(json["customFrom"].is_string()) 
        customFrom = json["customFrom"];

    // Load the keyframes
    for(auto& frame : json["keyFrames"]) {
        Frame newFrame;

        // Key Frame Properties
        if(frame["duration"].is_number_integer())       newFrame.duration = frame["duration"];
        if(frame["cancel"].is_boolean())                newFrame.cancel = frame["cancel"];
        if(frame["sound"].is_string())                  newFrame.sound = frame["sound"];

        if(frame["impulse"]["x"].is_number_float())     newFrame.impulse.x = frame["impulse"]["x"];
        if(frame["impulse"]["y"].is_number_float())     newFrame.impulse.y = frame["impulse"]["y"];

        // Load Joints
        int i = 0;
        for(auto& joint : frame["joints"]) {

            if(joint["x"].is_number_float())            newFrame.pose.joints[i].x = joint["x"];
            if(joint["y"].is_number_float())            newFrame.pose.joints[i].y = joint["y"];   

            i ++;
        }

        // Load Draw Order
        i = 0;
        for(auto& order : frame["order"]) {

            if(order.is_number_integer())               newFrame.pose.order[i] = order;

            i ++;
        }

        // Load HitBoxes
        for(auto& hitBox : frame["hitBoxes"]) {
            HitBox add;

            if(hitBox["x"].is_number_float())           add.x = hitBox["x"];
            if(hitBox["y"].is_number_float())           add.y = hitBox["y"];
            if(hitBox["w"].is_number_float())           add.w = hitBox["w"];
            if(hitBox["h"].is_number_float())           add.h = hitBox["h"];

            if(hitBox["damage"].is_number_integer())    add.damage = hitBox["damage"];
            if(hitBox["hitStun"].is_number_integer())   add.hitStun = hitBox["hitStun"];
            if(hitBox["blockStun"].is_number_integer()) add.blockStun = hitBox["blockStun"];
            if(hitBox["knockdown"].is_boolean())        add.knockdown = hitBox["knockdown"];

            if(hitBox["force"]["x"].is_number_float())  add.force.x = hitBox["force"]["x"]; 
            if(hitBox["force"]["y"].is_number_float())  add.force.y = hitBox["force"]["y"];
            
            newFrame.hitBoxes.push_back(add);
        }

        // Load HurtBoxes
        for(auto& hurtBox : frame["hurtBoxes"]) {
            HurtBox add;

            if(hurtBox["x"].is_number_float())          add.x = hurtBox["x"];
            if(hurtBox["y"].is_number_float())          add.y = hurtBox["y"];
            if(hurtBox["w"].is_number_float())          add.w = hurtBox["w"];
            if(hurtBox["h"].is_number_float())          add.h = hurtBox["h"]; 

            if(hurtBox["armour"].is_number_integer())   add.armour  = hurtBox["armour"];

            newFrame.hurtBoxes.push_back(add);
        }
        keyFrames.push_back(newFrame);
    }
    file.close();
}