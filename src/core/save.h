#ifndef GAME_SAVE_MANAGER_H
#define GAME_SAVE_MANAGER_H

#include <map>
#include <string>
#include <filesystem>
#include <SFML/Graphics.hpp>

#include "clothing.h"
#include "animation.h"

struct SaveManager {

	std::map<std::string, sf::Texture*> textures;
	std::map<std::string, Clothing*> clothes;
	std::map<std::string, Animation*> animations;

	SaveManager(std::filesystem::path path);
	~SaveManager();

	sf::Texture* getTexture(std::filesystem::path path);
	Clothing* getClothing(std::filesystem::path path);
	Animation* getAnimation(std::filesystem::path path);
};

namespace g {
	extern SaveManager save;
};

#endif