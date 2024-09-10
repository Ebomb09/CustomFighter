#ifndef GAME_ANIMATION_KEY_FRAME_H
#define GAME_ANIMATION_KEY_FRAME_H

#include "math.h"
#include "skeleton.h"
#include "move.h"

#include <vector>
#include <string>
#include <filesystem>

struct Frame {
    int duration        = 1;
    Skeleton pose       = Skeleton();
    Vector2 impulse     = Vector2(0, 0);
    bool cancel         = false;
    std::string sound   = "";

    std::vector<HitBox> hitBoxes;
    std::vector<HurtBox> hurtBoxes;
};

struct Animation {
    Animation();

    // What the animation is
    std::string name                = "";
    int category                    = MoveCategory::Normal;

    // How you can get into the animation
    bool from[MoveCategory::Total];
    std::string customFrom          = "";

    std::vector<Frame> keyFrames;

    bool loadFromFile(std::filesystem::path path);
    void saveToFile(std::filesystem::path path);

    void swapKeyFrame(int a, int b);

    void insertKeyFrame(int index, Frame k);
    void removeKeyFrame(int index);
    
    Frame& getKeyFrame(int index);
    int getKeyFrameCount();

    Frame getFrame(int frame);
    int getFrameCount();

    int getStartup();
    int getDamage();
    int getOnHit();
    int getOnBlock();
};

#endif