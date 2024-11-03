#include "animated_texture.h"
#include "save.h"

#include <cmath>

bool AnimatedTexture::loadFromFile(std::filesystem::path path) {

    if(!std::filesystem::is_directory(path))
        return false;

    int i = 1;
    sf::Texture* tex = NULL;

    do {
        tex = g::save.getTexture(path.string() + "/" + std::to_string(i) + ".png");

        if(tex) 
            textures.push_back(tex);

        i ++;

    }while(tex);

    return (textures.size() > 0);
}

sf::Texture* AnimatedTexture::getFrame(int current, int length) {

    if(textures.size() < 0)
        return NULL;

    float prog = (textures.size()-1) * (float)current / (float)length;
    int i = std::clamp((int)std::round(prog), 0, (int)textures.size()-1);
    return textures[i];
}

int AnimatedTexture::getFrameCount() {
    return textures.size();
}