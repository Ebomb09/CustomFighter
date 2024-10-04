#ifndef GAME_ANIMATED_TEXTURE_H
#define GAME_ANIMATED_TEXTURE_H

#include <filesystem>
#include <vector>
#include <SFML/Graphics.hpp>

struct AnimatedTexture {
    int id = -1;
    std::vector<sf::Texture*> textures;

    bool loadFromFile(std::filesystem::path path);
    sf::Texture* getFrame(int current, int length);
    int getFrameCount();
};

#endif