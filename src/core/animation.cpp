#include "animation.h"
#include "move.h"

#include <fstream>
#include <json.hpp>
#include <iostream>

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

            frame.key = i;

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

                // Interpolate grab
                if(keyFrames[i].isGrab && keyFrames[i + 1].isGrab) {
                    Skeleton poseA = keyFrames[i].grabee;                
                    Skeleton poseB = keyFrames[i + 1].grabee;

                    for(int j = 0; j < poseB.jointCount; j ++) {
                        Vector2 mov = (poseB.joints[j] - poseA.joints[j]) * progress;
                        frame.grabee.joints[j] += mov;
                    }
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

int Animation::getStartup() {
    int f_pos = 0;

    for(auto& frame : keyFrames) {

        if(frame.hitBoxes.size() > 0)
            return f_pos;

        f_pos += frame.duration;
    }
    return -1;
}

int Animation::getDamage() {
    int dmg = 0;

    for(auto& frame : keyFrames) {
        int max = 0;

        for(auto& hit : frame.hitBoxes) {
            max = std::max(max, hit.damage);
        }
        dmg += max;
    }
    return dmg;
}

int Animation::getOnHit() {
    int last_hit = 0;
    int f_pos = 0;

    for(auto& frame : keyFrames) {
        int max = 0;

        for(auto& hit : frame.hitBoxes) {
            max = std::max(max, hit.hitStun);
        }

        if(max > 0) {
            last_hit = max;
            f_pos = 0;
        }

        f_pos += frame.duration;
    }
    return last_hit - f_pos;
}

int Animation::getOnBlock() {
    int last_hit = 0;
    int f_pos = 0;

    for(auto& frame : keyFrames) {
        int max = 0;

        for(auto& hit : frame.hitBoxes) {
            max = std::max(max, hit.blockStun);
        }

        if(max > 0) {
            last_hit = max;
            f_pos = 0;
        }

        f_pos += frame.duration;
    }
    return last_hit - f_pos;
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
        newFrame["isGrab"]   = keyFrames[i].isGrab;

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

        if(keyFrames[i].isGrab) {
            newFrame["grabee"] = nlohmann::json::array();

            for(int j = 0; j < keyFrames[i].grabee.jointCount; j ++) {
                nlohmann::json obj;
                obj["x"] = keyFrames[i].grabee.joints[j].x;
                obj["y"] = keyFrames[i].grabee.joints[j].y;      
                newFrame["grabee"].push_back(obj);      
            } 
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
            obj["type"]        = keyFrames[i].hitBoxes[j].type;
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

bool Animation::loadFromFile(std::filesystem::path path) {
    keyFrames.clear();

    std::fstream file(path, std::fstream::in);

    if(!file.good()) {
        file.close();
        return false;
    }

    // Parse json
    nlohmann::json json;

    try {
        json = nlohmann::json::parse(file);

    }catch(nlohmann::json::exception e) {
        return false;
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
        if(frame["isGrab"].is_boolean())                newFrame.isGrab = frame["isGrab"];

        if(frame["impulse"]["x"].is_number_float())     newFrame.impulse.x = frame["impulse"]["x"];
        if(frame["impulse"]["y"].is_number_float())     newFrame.impulse.y = frame["impulse"]["y"];

        // Load Joints
        int i = 0;
        for(auto& joint : frame["joints"]) {

            if(joint["x"].is_number_float())            newFrame.pose.joints[i].x = joint["x"];
            if(joint["y"].is_number_float())            newFrame.pose.joints[i].y = joint["y"];   

            i ++;
        }

        if(newFrame.isGrab) {
            i = 0;
            for(auto& joint : frame["grabee"]) {

                if(joint["x"].is_number_float())        newFrame.grabee.joints[i].x = joint["x"];
                if(joint["y"].is_number_float())        newFrame.grabee.joints[i].y = joint["y"];   

                i ++;
            }
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

            if(hitBox["type"].is_number_integer())      add.type = hitBox["type"];
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
    return true;
}

static Vector2 getPos(nlohmann::json& obj) {

    if(obj.is_object() && obj["x"].is_number() && obj["z"].is_number())
        return {obj["x"], obj["z"]};

    return {0, 0};
}

bool Animation::importFromFreeMoCap(std::filesystem::path path) {
    keyFrames.clear();

    std::fstream file(path, std::fstream::in);

    if(!file.good()) {
        file.close();
        return false;
    }

    // Parse json
    nlohmann::json json;

    try {
        json = nlohmann::json::parse(file);

    }catch(nlohmann::json::exception e) {
        return false;
    }

    if(json["data_by_frame"].is_object()) {
        int frame = 0;

        while(json["data_by_frame"][std::to_string(frame)]["tracked_points"].is_object()) {
            nlohmann::json& obj = json["data_by_frame"][std::to_string(frame)]["tracked_points"];

            Frame add;
            Skeleton& pose = add.pose;
            pose.head           = getPos(obj["head"]);
            pose.shoulder[0]    = getPos(obj["body_left_shoulder"]);
            pose.shoulder[1]    = getPos(obj["body_right_shoulder"]);
            pose.elbow[0]       = getPos(obj["body_left_elbow"]);
            pose.elbow[1]       = getPos(obj["body_right_elbow"]);
            pose.wrist[0]       = getPos(obj["body_left_wrist"]);
            pose.wrist[1]       = getPos(obj["body_right_wrist"]);
            pose.hip[0]         = getPos(obj["body_left_hip"]);
            pose.hip[1]         = getPos(obj["body_right_hip"]);
            pose.knee[0]        = getPos(obj["body_left_knee"]);
            pose.knee[1]        = getPos(obj["body_right_knee"]);
            pose.heel[0]        = getPos(obj["body_left_heel"]);
            pose.heel[1]        = getPos(obj["body_right_heel"]);
            pose.toes[0]        = getPos(obj["body_left_foot_index"]);
            pose.toes[1]        = getPos(obj["body_right_foot_index"]);
            pose.fingers[0]     = getPos(obj["left_hand"]);
            pose.fingers[1]     = getPos(obj["right_hand"]);

            pose.torso[0] = (pose.shoulder[0] + pose.shoulder[1]) / 2.f;
            pose.torso[1] = (pose.hip[0] + pose.hip[1]) / 2.f;

            // Adjust the skeleton
            float bot = std::min(pose.heel[0].y, pose.heel[1].y);
            float mid = pose.torso[1].x;

            for(int i = 0; i < pose.jointCount; i ++) {
                pose.joints[i].y += -1.f * bot;
                pose.joints[i].x -= mid;
                pose.joints[i] /= 12.5f;
            }

            pose.head = pose.torso[0].translate((pose.head - pose.torso[0]).getAngle(), 4.f);

            keyFrames.push_back(add);

            frame ++;
        }
    }
    file.close();
    return true;
}