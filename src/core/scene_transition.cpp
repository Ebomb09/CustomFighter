#include "scene_transition.h"

#include <cmath>
#include <algorithm>

void SceneTransition::advanceFrame() {
    transitionCounter ++;

    switch(transitionType) {
        case Close: if(ready()) { sceneCurrent = sceneNext; } break;
        case Open: if(ready()) { sceneCurrent = sceneNext; } break;
        case CloseThenOpen: if(transitionCounter >= transitionMaxCounter / 2) { sceneCurrent = sceneNext; } break;
    }
}

void SceneTransition::nextScene(int _type, int _next) {
    transitionCounter = 0;
    transitionType = _type;
    sceneNext = _next;

    switch(transitionType) {

        case None: 
            transitionMaxCounter = 0; 
            sceneCurrent = sceneNext;
            break;

        case Close: 
            transitionMaxCounter = 30; 
            break;

        case Open: 
            transitionMaxCounter = 30; 
            sceneCurrent = sceneNext;
            break;

        case CloseThenOpen: 
            transitionMaxCounter = 60; 
            break;
    }
}

bool SceneTransition::ready() {
    return transitionCounter >= transitionMaxCounter;
}

float SceneTransition::percent() {

    // Close -> Open
    float pct = (float)std::clamp(counter(), 0, transitionMaxCounter) / (float)transitionMaxCounter;

    switch(transitionType) {

    // Open -> Close
    case Close:
        pct = 1.f - pct;
        break;

    // Open -> Close -> Open
    case CloseThenOpen:

        // Open -> Close
        if(pct < 0.5f) {
            pct = 1.f - pct * 2.f;

        // Close -> Open
        }else {
            pct = (pct - 0.5f) * 2.f;
        }
        break;
    }
    return pct;
}

int SceneTransition::counter() {
    return transitionCounter;
}

int SceneTransition::scene() {
    return sceneCurrent;
}

Rectangle SceneTransition::getGrowthEffect(const Rectangle& open, const Rectangle& close) {

    return Rectangle{
        close.x + (open.x - close.x) * percent(),
        close.y + (open.y - close.y) * percent(),
        close.w + (open.w - close.w) * percent(),
        close.h + (open.h - close.h) * percent()
    };         
}

sf::CircleShape SceneTransition::getCircleFadeEffect(const Rectangle& area) {
    float maxSize = std::sqrt(std::pow(area.w, 2) + std::pow(area.h, 2));

    sf::CircleShape circ;
    circ.setFillColor(sf::Color::Transparent);
    circ.setOutlineColor(sf::Color::Black);
    circ.setOutlineThickness(maxSize);

    // Radius needs to be clamped to just above 0
    float radius = std::clamp(percent() * maxSize, 0.001f, maxSize);

    circ.setRadius(radius);
    circ.setPosition({area.x + area.w / 2.f - radius, area.y + area.h / 2.f - radius});

    return circ;
}