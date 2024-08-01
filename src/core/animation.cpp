#include <fstream>
#include <json.hpp>
#include "animation.h"

Frame::Frame() : pose(), duration(1) {}

Frame::Frame(Skeleton _pose, int _duration) : pose(_pose), duration(_duration) {}

Frame::Frame(const Frame& k) { 
    *this = k; 
}    

Frame::Frame(Frame&& k) { 
    *this = k; 
}

Frame& Frame::operator=(const Frame& copy) { 
    pose = copy.pose; 
    duration = copy.duration;
    hitBoxes = copy.hitBoxes;
    hurtBoxes = copy.hurtBoxes;
    return *this; 
}

Frame& Frame::operator=(Frame&& move) { 
    pose = std::move(move.pose); 
    duration = std::move(move.duration);
    hitBoxes = std::move(move.hitBoxes);
    hurtBoxes = std::move(move.hurtBoxes);    
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
            Skeleton poseA = keyFrames[i].pose;

            // If there's a next frame interpolate
            if(i + 1 < getKeyFrameCount()) {
                Skeleton poseB = keyFrames[i + 1].pose;
                float progress = (t - f_pos) / (float)keyFrames[i].duration;

                // Calculate (final - initial) and times by frame progression
                for(int j = 0; j < poseB.jointCount; j ++) {
                    Vector2 mov = (poseB.joints[j] - poseA.joints[j]) * progress;
                    poseA.joints[j] += mov;
                }
            }

            frame.pose = poseA;
            frame.hitBoxes = keyFrames[i].hitBoxes;
            frame.hurtBoxes = keyFrames[i].hurtBoxes;
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

    json["inCrouch"] = inCrouch;
    json["inStand"] = inStand;
    json["inJump"] = inJump;

    json["keyFrames"] = nlohmann::json::array();


    for(int i = 0; i < keyFrames.size(); i ++) {
        // Duration
        json["keyFrames"][i]["duration"] = keyFrames[i].duration;

        json["keyFrames"][i]["joints"] = nlohmann::json::array();

        for(int j = 0; j < keyFrames[i].pose.jointCount; j ++) {
            auto& node = json["keyFrames"][i]["joints"][j];
            node["x"] = keyFrames[i].pose.joints[j].x;
            node["y"] = keyFrames[i].pose.joints[j].y;            
        } 

        json["keyFrames"][i]["hitBoxes"] = nlohmann::json::array();   

        for(int j = 0; j < keyFrames[i].hitBoxes.size(); j ++) {
            auto& node = json["keyFrames"][i]["hitBoxes"][j];
            node["x"] = keyFrames[i].hitBoxes[j].x;
            node["y"] = keyFrames[i].hitBoxes[j].y;
            node["w"] = keyFrames[i].hitBoxes[j].w;
            node["h"] = keyFrames[i].hitBoxes[j].h;
            node["damage"] = keyFrames[i].hitBoxes[j].damage;
            node["hitStun"] = keyFrames[i].hitBoxes[j].hitStun;
            node["blockStun"] = keyFrames[i].hitBoxes[j].blockStun;
            node["knockdown"] = keyFrames[i].hitBoxes[j].knockdown;
            node["impulse"]["x"] = keyFrames[i].hitBoxes[j].impulse.x;
            node["impulse"]["y"] = keyFrames[i].hitBoxes[j].impulse.y;            
        } 

        json["keyFrames"][i]["hurtBoxes"] = nlohmann::json::array();   

        for(int j = 0; j < keyFrames[i].hurtBoxes.size(); j ++) {
            auto& node = json["keyFrames"][i]["hurtBoxes"][j];
            node["x"] = keyFrames[i].hurtBoxes[j].x;
            node["y"] = keyFrames[i].hurtBoxes[j].y;
            node["w"] = keyFrames[i].hurtBoxes[j].w;
            node["h"] = keyFrames[i].hurtBoxes[j].h;
            node["armour"] = keyFrames[i].hurtBoxes[j].armour;
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

    nlohmann::json json = nlohmann::json::parse(file);

    inCrouch = json["inCrouch"];
    inStand = json["inStand"];    
    inJump = json["inJump"];

    auto& keyFramesAr = json["keyFrames"];

    for(int i = 0; i < keyFramesAr.size(); i ++) {
        Frame newFrame;
        newFrame.duration = keyFramesAr[i]["duration"];

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

            if(hitBoxAr[j]["x"].is_number_float())
                add.x = hitBoxAr[j]["x"];

            if(hitBoxAr[j]["y"].is_number_float())
                add.y = hitBoxAr[j]["y"];

            if(hitBoxAr[j]["w"].is_number_float())            
                add.w = hitBoxAr[j]["w"];

            if(hitBoxAr[j]["h"].is_number_float())
                add.h = hitBoxAr[j]["h"];  

            if(hitBoxAr[j]["damage"].is_number_integer())         
                add.damage = hitBoxAr[j]["damage"];

            if(hitBoxAr[j]["hitStun"].is_number_integer())              
                add.hitStun = hitBoxAr[j]["hitStun"];

            if(hitBoxAr[j]["blockStun"].is_number_integer())             
                add.blockStun = hitBoxAr[j]["blockStun"];

            if(hitBoxAr[j]["knockdown"].is_boolean())    
                add.knockdown = hitBoxAr[j]["knockdown"];

            if(hitBoxAr[j]["impulse"].is_object()) {

                if(hitBoxAr[j]["impulse"]["x"].is_number_float())
                    add.impulse.x = hitBoxAr[j]["impulse"]["x"];

                if(hitBoxAr[j]["impulse"]["y"].is_number_float())                
                    add.impulse.y = hitBoxAr[j]["impulse"]["y"];            
            }
            
            newFrame.hitBoxes.push_back(add);
        }

        for(int j = 0; j < hurtBoxAr.size(); j++) {
            HurtBox add;

            if(hurtBoxAr[j]["x"].is_number_float())
                add.x = hurtBoxAr[j]["x"];

            if(hurtBoxAr[j]["y"].is_number_float())
                add.y = hurtBoxAr[j]["y"];

            if(hurtBoxAr[j]["w"].is_number_float())            
                add.w = hurtBoxAr[j]["w"];

            if(hurtBoxAr[j]["h"].is_number_float())
                add.h = hurtBoxAr[j]["h"]; 

            if(hurtBoxAr[j]["armour"].is_number_integer())                        
                add.armour = hurtBoxAr[j]["armour"];

            newFrame.hurtBoxes.push_back(add);
        }    
        keyFrames.push_back(newFrame);
    }
    file.close();
}