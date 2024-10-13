#ifndef GAME_STAGE_H
#define GAME_STAGE_H

#include "video.h"

#include <vector>
#include <filesystem>

#include <SFML/Graphics.hpp>

const Rectangle StageBounds = {
	-384,
	224,
	768,
	256
};

const float StageLeft = StageBounds.x + 16;
const float StageRight = StageBounds.x + StageBounds.w - 16;

const Rectangle CameraBounds = {
	0, 
	0,
	256,
	192
};

struct Stage {

    struct Layer {
        sf::Texture* tex;
        float scale;
    };

    std::vector<Layer> layers;

    void draw(Renderer* renderer = NULL);

    bool loadFromFile(std::filesystem::path path);
};

#endif