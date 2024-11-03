#include "scene_transition.h"

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
    return (float)std::clamp(counter(), 0, transitionMaxCounter) / (float)transitionMaxCounter;
}

int SceneTransition::counter() {
    return transitionCounter;
}

int SceneTransition::scene() {
    return sceneCurrent;
}

Rectangle SceneTransition::getGrowthEffect(const Rectangle& open, const Rectangle& close) {

    switch(transitionType) {

        case Open: 
            return Rectangle{
                close.x + (open.x - close.x) * percent(),
                close.y + (open.y - close.y) * percent(),
                close.w + (open.w - close.w) * percent(),
                close.h + (open.h - close.h) * percent()
            };

        case Close:
            return Rectangle{
                open.x + (close.x - open.x) * percent(),
                open.y + (close.y - open.y) * percent(),
                open.w + (close.w - open.w) * percent(),
                open.h + (close.h - open.h) * percent()
            };

        case CloseThenOpen:
            if(percent() < 0.5f) {
                return Rectangle{
                    open.x + (close.x - open.x) * (percent() * 2.f),
                    open.y + (close.y - open.y) * (percent() * 2.f),
                    open.w + (close.w - open.w) * (percent() * 2.f),
                    open.h + (close.h - open.h) * (percent() * 2.f)
                };
            }else {
                return Rectangle{
                    close.x + (open.x - close.x) * ((percent() - 0.5f) * 2.f),
                    close.y + (open.y - close.y) * ((percent() - 0.5f) * 2.f),
                    close.w + (open.w - close.w) * ((percent() - 0.5f) * 2.f),
                    close.h + (open.h - close.h) * ((percent() - 0.5f) * 2.f)
                };         
            }
    }
    return {};
}