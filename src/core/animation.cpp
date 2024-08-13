#include "animation.h"
#include "move.h"

#include <fstream>
#include <json.hpp>

Frame::Frame() : pose(), duration(1) {}

Frame::Frame(Skeleton _pose, int _duration) : pose(_pose), duration(_duration) {}

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

    for(int i = 0; i < MoveCategory::Total; i ++) {
        json["category" + MoveCategory::String[i]] = category[i];
    }

    for(int i = 0; i < Move::Total; i ++) {
        json["from" + Move::String[i]] = from[i];
    }

    json["keyFrames"] = nlohmann::json::array();


    for(int i = 0; i < keyFrames.size(); i ++) {
        // Duration
        json["keyFrames"][i]["duration"] = keyFrames[i].duration;

        json["keyFrames"][i]["impulse"]["x"] = keyFrames[i].impulse.x;
        json["keyFrames"][i]["impulse"]["y"] = keyFrames[i].impulse.y;

        json["keyFrames"][i]["joints"] = nlohmann::json::array();

        json["keyFrames"][i]["cancel"] = keyFrames[i].cancel;

        for(int j = 0; j < keyFrames[i].pose.jointCount; j ++) {
            auto& node = json["keyFrames"][i]["joints"][j];
            node["x"] = keyFrames[i].pose.joints[j].x;
            node["y"] = keyFrames[i].pose.joints[j].y;            
        } 

        json["keyFrames"][i]["hitBoxes"] = nlohmann::json::array();   

        for(int j = 0; j < keyFrames[i].hitBoxes.size(); j ++) {
            auto& node = json["keyFrames"][i]["hitBoxes"][j];
            node["x"]           = keyFrames[i].hitBoxes[j].x;
            node["y"]           = keyFrames[i].hitBoxes[j].y;
            node["w"]           = keyFrames[i].hitBoxes[j].w;
            node["h"]           = keyFrames[i].hitBoxes[j].h;
            node["damage"]      = keyFrames[i].hitBoxes[j].damage;
            node["hitStun"]     = keyFrames[i].hitBoxes[j].hitStun;
            node["blockStun"]   = keyFrames[i].hitBoxes[j].blockStun;
            node["knockdown"]   = keyFrames[i].hitBoxes[j].knockdown;
            node["force"]["x"]  = keyFrames[i].hitBoxes[j].force.x;
            node["force"]["y"]  = keyFrames[i].hitBoxes[j].force.y;          
        } 

        json["keyFrames"][i]["hurtBoxes"] = nlohmann::json::array();   

        for(int j = 0; j < keyFrames[i].hurtBoxes.size(); j ++) {
            auto& node = json["keyFrames"][i]["hurtBoxes"][j];
            node["x"]       = keyFrames[i].hurtBoxes[j].x;
            node["y"]       = keyFrames[i].hurtBoxes[j].y;
            node["w"]       = keyFrames[i].hurtBoxes[j].w;
            node["h"]       = keyFrames[i].hurtBoxes[j].h;
            node["armour"]  = keyFrames[i].hurtBoxes[j].armour;
        } 
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

    nlohmann::json json;

    try {
        json = nlohmann::json::parse(file);

    }catch(nlohmann::json::exception e) {
        return;
    }

    // Load categories
    for(int i = 0; i < MoveCategory::Total; i ++) {
        category[i] = json.value("category" + MoveCategory::String[i], false);
    }

    for(int i = 0; i < Move::Total; i ++) {
        from[i] = json.value("from" + Move::String[i], false);
    }

    auto& keyFramesAr = json["keyFrames"];

    for(int i = 0; i < keyFramesAr.size(); i ++) {
        Frame newFrame;

        newFrame.duration = keyFramesAr[i].value("duration", 0);
        newFrame.cancel = keyFramesAr[i].value("cancel", "");

        if(keyFramesAr[i]["impulse"].is_object()) {
            newFrame.impulse.x = keyFramesAr[i]["impulse"].value("x", 0);
            newFrame.impulse.y = keyFramesAr[i]["impulse"].value("y", 0);
        }

        auto& jointAr = keyFramesAr[i]["joints"];
        auto& hitBoxAr = keyFramesAr[i]["hitBoxes"];
        auto& hurtBoxAr = keyFramesAr[i]["hurtBoxes"];

        for(int j = 0; j < jointAr.size(); j++) {

            if(jointAr[j]["x"].is_number_float())
                newFrame.pose.joints[j].x = jointAr[j]["x"];

            if(jointAr[j]["y"].is_number_float())
                newFrame.pose.joints[j].y = jointAr[j]["y"];   
        }

        for(int j = 0; j < hitBoxAr.size(); j++) {
            HitBox add;
            add.x           = hitBoxAr[j].value("x", 0);
            add.y           = hitBoxAr[j].value("y", 0);
            add.w           = hitBoxAr[j].value("w", 0);
            add.h           = hitBoxAr[j].value("h", 0);  
            add.damage      = hitBoxAr[j].value("damage", 0);
            add.hitStun     = hitBoxAr[j].value("hitStun", 0);
            add.blockStun   = hitBoxAr[j].value("blockStun", 0);
            add.knockdown   = hitBoxAr[j].value("knockdown", false);

            if(hitBoxAr[j]["force"].is_object()) {
                add.force.x = hitBoxAr[j]["force"].value("x", 0);             
                add.force.y = hitBoxAr[j]["force"].value("y", 0);            
            }
            
            newFrame.hitBoxes.push_back(add);
        }

        for(int j = 0; j < hurtBoxAr.size(); j++) {
            HurtBox add;
            add.x       = hurtBoxAr[j].value("x", 0);
            add.y       = hurtBoxAr[j].value("y", 0);
            add.w       = hurtBoxAr[j].value("w", 0);
            add.h       = hurtBoxAr[j].value("h", 0); 
            add.armour  = hurtBoxAr[j].value("armour", 0);

            newFrame.hurtBoxes.push_back(add);
        }    
        keyFrames.push_back(newFrame);
    }
    file.close();
}