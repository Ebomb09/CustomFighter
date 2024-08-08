#ifndef GAME_SAVE_MANAGER_H
#define GAME_SAVE_MANAGER_H

#include <map>
#include <string>
#include <filesystem>
#include <SFML/Graphics.hpp>

#include "clothing.h"
#include "animation.h"
#include "button.h"
#include "player.h"

struct SaveManager {

	std::map<std::string, sf::Texture*> textures;
	std::map<std::string, Clothing*> clothes;
	std::map<std::string, Animation*> animations;

	Button::Config buttonConfig[2];
	Player::Config playerConfig[2];

	std::vector<std::string> serverList;
	string ip;
	int port;

	SaveManager();
	~SaveManager();

	sf::Texture* getTexture(std::filesystem::path path);
	Clothing* getClothing(std::filesystem::path path);
	Animation* getAnimation(std::filesystem::path path);

	void loadButtonConfig(int index);
	void saveButtonConfig(int index, Button::Config config);
	Button::Config getButtonConfig(int index);

	void loadPlayerConfig(int index);
	void savePlayerConfig(int index, Player::Config config);
	Player::Config getPlayerConfig(int index);

	void loadServerList();

	string getNetworkAddress();
};

namespace g {
	extern SaveManager save;
};

#endif