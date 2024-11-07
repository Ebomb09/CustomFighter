#ifndef GAME_SCENE_TRANSITION_H
#define GAME_SCENE_TRANSITION_H

#include "math.h"
#include "SFML/Graphics.hpp"

class SceneTransition {

public:
    enum Type {
        None,
        Close,
        Open,
        CloseThenOpen
    };

    void advanceFrame();
    void nextScene(int _type, int _next);
    bool ready();
    float percent();
    int counter();
    int scene();

    Rectangle getGrowthEffect(const Rectangle& open, const Rectangle& close);
    sf::CircleShape getCircleFadeEffect(const Rectangle& area);

private:

    // Transition state control
    int transitionType = None;
    int transitionCounter = 0;
    int transitionMaxCounter = 0;

    // Scenes
    int sceneCurrent = 0;
    int sceneNext = 0;
};

#endif