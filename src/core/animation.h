#ifndef GAME_ANIMATION_KEY_FRAME_H
#define GAME_ANIMATION_KEY_FRAME_H

#include "math.h"
#include "skeleton.h"
#include "move.h"

#include <vector>
#include <string>

struct Frame {
    int duration;
    Skeleton pose;
    Vector2 impulse;
    std::string cancel;

    std::vector<HitBox> hitBoxes;
    std::vector<HurtBox> hurtBoxes;

    Frame();
    Frame(Skeleton _pose, int _duration);
    Frame(const Frame& k);   
    Frame(Frame&& k);

    Frame& operator=(const Frame& copy);
    Frame& operator=(Frame&& move);
};

struct Animation {

    std::string name;

    // Organizational
    bool category[MoveCategory::Total];

    // Leading animations can be cancelled from
    bool from[Move::Total];

    std::vector<Frame> keyFrames;

    void loadFromFile(std::string fileName);
    void saveToFile(std::string fileName);

    void swapKeyFrame(int a, int b);

    void insertKeyFrame(int index, Frame k);
    void removeKeyFrame(int index);
    
    Frame& getKeyFrame(int index);
    int getKeyFrameCount();

    Frame getFrame(float frame);
    int getFrameCount();
};

#endif