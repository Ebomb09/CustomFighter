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
	std::map<std::string, sf::Font*> fonts;
	std::map<std::string, Clothing*> clothes;
	std::map<std::string, Animation*> animations;

	static const int maxButtonConfigs = 4;
	Button::Config buttonConfig[maxButtonConfigs];

	static const int maxPlayerConfigs = 30;	
	Player::Config playerConfig[maxPlayerConfigs];

	std::vector<std::string> serverList;
	string ip;
	int port;

	SaveManager();
	~SaveManager();

	sf::Texture* getTexture(std::filesystem::path path);

	Clothing* getClothing(std::filesystem::path path);

	Animation* getAnimation(std::filesystem::path path);
	std::vector<Animation*> getAnimationsByFilter(int category);

	sf::Font* getFont(std::filesystem::path path);

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