#include "save.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <json.hpp>
#include <curl/curl.h>

SaveManager g::save = SaveManager();

size_t write_callbackz(char *ptr, size_t size, size_t nmemb, void *userdata) {
    string* write_data = (string*)userdata; 
    *write_data += string(ptr, nmemb);
    return nmemb;
}

SaveManager::SaveManager() {
	std::srand(std::time(0));
	
	for(auto& entry : std::filesystem::directory_iterator("data/clothing"))
		getClothing(entry);

	for(auto& entry : std::filesystem::directory_iterator("data/moves"))
		getAnimation(entry);

	// Animation used for testing purposes
	animations["test"] = new Animation();

	for(auto& entry : std::filesystem::directory_iterator("data/fonts"))
		getFont(entry);

	for(auto& entry : std::filesystem::directory_iterator("data/stages")) {
		sf::Texture* ptr = getTexture(entry);

		if(ptr) 
			stages.push_back(ptr);
	}

	for(int i = 0; i < maxButtonConfigs; i ++)
		loadButtonConfig(i);

	for(int i = 0; i < maxPlayerConfigs; i ++)
		loadPlayerConfig(i);

	loadServerList();

	// Get network address
	port = 50000 + std::rand() % 1000;

    CURL* curl = curl_easy_init();
    if(curl) {
        string write_data = "";
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org/");	

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, false);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callbackz);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_data);

        if(curl_easy_perform(curl) == CURLE_OK) {
        	ip = write_data;
        }
        curl_easy_cleanup(curl);
    }
}

SaveManager::~SaveManager() {

	for(auto& it : textures)
		delete it.second;

	for(auto& it : clothes)
		delete it.second;

	for(auto& it : animations)
		delete it.second;
}

sf::Texture* SaveManager::getTexture(std::filesystem::path path) {
	sf::Texture* ptr = NULL;

	if(textures.find(path.string()) == textures.end()) {
		ptr = new sf::Texture();

		if(ptr->loadFromFile(path.string())) {
			textures[path.string()] = ptr;

		}else {
			delete ptr;
			ptr = NULL;
		}

	}else {
		ptr = textures[path.string()];
	}
	return ptr;
}

sf::Font* SaveManager::getFont(std::filesystem::path path) {
	sf::Font* ptr = NULL;

	if(fonts.find(path.string()) == fonts.end()) {
		ptr = new sf::Font();

		if(ptr->loadFromFile(path.string())) {
			fonts[path.stem().string()] = ptr;

		}else {
			delete ptr;
			ptr = NULL;
		}

	}else {
		ptr = fonts[path.string()];
	}
	return ptr;
}

Clothing* SaveManager::getClothing(std::filesystem::path path) {
	Clothing* ptr = NULL;

	if(clothes.find(path.string()) == clothes.end()) {

		// Find from list by name
		for(auto& clothing : getClothingList()) {
			if(clothing->name == path.string()) {
				ptr = clothing;
			}
		}

		// Load from file
		if(!ptr) {
			ptr = new Clothing;
			ptr->name		= path.stem().string();
			ptr->torsoFront	= getTexture(path/"torsoFront.png");
			ptr->torsoBack	= getTexture(path/"torsoBack.png");
			ptr->neck 		= getTexture(path/"neck.png");
			ptr->upperArm 	= getTexture(path/"upperArm.png");
			ptr->foreArm 	= getTexture(path/"foreArm.png");
			ptr->pelvis 	= getTexture(path/"pelvis.png");
			ptr->thigh 		= getTexture(path/"thigh.png");
			ptr->calf 		= getTexture(path/"calf.png");
			ptr->hand 		= getTexture(path/"hand.png");	
			ptr->foot 		= getTexture(path/"foot.png");
			ptr->head 		= getTexture(path/"head.png");
			clothes[path.string()] = ptr;			
		}
	
	}else {
		ptr = clothes.at(path.string());
	}
	return ptr;
}

vector<Clothing*> SaveManager::getClothingList() {
	vector<Clothing*> list;

    for(auto it = g::save.clothes.begin(); it != g::save.clothes.end(); it ++) 
        list.push_back(it->second);
    
    return list;
}

Animation* SaveManager::getAnimation(std::filesystem::path path) {
	Animation* ptr = NULL;

	if(animations.find(path.string()) == animations.end()) {
		ptr = new Animation();
		ptr->loadFromFile(path.string());
		animations[path.stem().string()] = ptr;	

	}else {
		ptr = animations.at(path.string());		
	}

	return ptr;	
}

vector<Animation*> SaveManager::getAnimationsList() {
	vector<Animation*> list;

    for(auto it = g::save.animations.begin(); it != g::save.animations.end(); it ++) 
        list.push_back(it->second);
    
    return list;
}

vector<Animation*> SaveManager::getAnimationsByFilter(std::vector<int> filters) {
	vector<Animation*> out;

	for(std::pair<string, Animation*> it : animations) {

		if(std::find(filters.begin(), filters.end(), it.second->category) != filters.end())
			out.push_back(it.second);
	}
	return out;
}

void SaveManager::loadButtonConfig(int index) {
	std::fstream file("save/inputPlayer" + std::to_string(index) + ".json", std::fstream::in);

	if(!file.good()) {
		file.close();

		if(index == 0) {
			buttonConfig[index].Up 		= sf::Keyboard::W;
			buttonConfig[index].Down 	= sf::Keyboard::S;
			buttonConfig[index].Left 	= sf::Keyboard::A;
			buttonConfig[index].Right 	= sf::Keyboard::D;
			buttonConfig[index].A 		= sf::Keyboard::U;
			buttonConfig[index].B 		= sf::Keyboard::J;
			buttonConfig[index].C 		= sf::Keyboard::I;
			buttonConfig[index].D 		= sf::Keyboard::O;
			buttonConfig[index].Taunt 	= sf::Keyboard::Semicolon;		
		}

		if(index == 1) {
			buttonConfig[index].Up 		= sf::Keyboard::Up;
			buttonConfig[index].Down 	= sf::Keyboard::Down;
			buttonConfig[index].Left 	= sf::Keyboard::Left;
			buttonConfig[index].Right 	= sf::Keyboard::Right;
			buttonConfig[index].A 		= sf::Keyboard::Numpad4;
			buttonConfig[index].B 		= sf::Keyboard::Numpad1;
			buttonConfig[index].C 		= sf::Keyboard::Numpad5;
			buttonConfig[index].D 		= sf::Keyboard::Numpad6;
			buttonConfig[index].Taunt 	= sf::Keyboard::Enter;
		}

		return;
	}

	nlohmann::json json = nlohmann::json::parse(file);
	buttonConfig[index].Up 		= json["Up"];
	buttonConfig[index].Down 	= json["Down"];
	buttonConfig[index].Left 	= json["Left"];
	buttonConfig[index].Right 	= json["Right"];
	buttonConfig[index].A 		= json["A"];
	buttonConfig[index].B 		= json["B"];
	buttonConfig[index].C 		= json["C"];
	buttonConfig[index].D 		= json["D"];
	buttonConfig[index].Taunt 	= json["Taunt"];

	file.close();
	return;
}

void SaveManager::saveButtonConfig(int index, Button::Config config) {
	buttonConfig[index] = config;
	std::fstream file("save/inputPlayer" + std::to_string(index) + ".json", std::fstream::out);

	nlohmann::json json;
	json["Up"] 		= buttonConfig[index].Up;
	json["Down"] 	= buttonConfig[index].Down; 
	json["Left"] 	= buttonConfig[index].Left; 
	json["Right"] 	= buttonConfig[index].Right;
	json["A"] 		= buttonConfig[index].A; 	
	json["B"] 		= buttonConfig[index].B; 	
	json["C"] 		= buttonConfig[index].C; 	
	json["D"] 		= buttonConfig[index].D; 	
	json["Taunt"] 	= buttonConfig[index].Taunt;

	file.close();
}

Button::Config SaveManager::getButtonConfig(int index) {
	return buttonConfig[index];
}

void SaveManager::loadPlayerConfig(int index) {
	std::string path = "save/player" + std::to_string(index) + ".json";
	playerConfig[index].loadFromFile(path);
}

void SaveManager::savePlayerConfig(int index, Player::Config config) {
	std::string path = "save/player" + std::to_string(index) + ".json";
	playerConfig[index] = config;
	playerConfig[index].saveToFile(path);
}

Player::Config SaveManager::getPlayerConfig(int index) {	
	return playerConfig[index];
}

void SaveManager::loadServerList() {
	std::fstream file("save/serverList", std::fstream::in);

	if(!file.good()) {
		file.close();
		return;
	}

	std::string line;
	do{
		std::getline(file, line);
		serverList.push_back(line);

	}while(!file.eof());

	file.close();
}

string SaveManager::getServer(int index) {
	index = std::clamp(index, 0, (int)serverList.size()-1);
	return serverList[index];
}

string SaveManager::getNetworkAddress() {
	return ip + ":" + std::to_string(port);
}

int SaveManager::getPort() {
	return port;
}

int SaveManager::getRandomStage() {
	return rand() % stages.size();
}

sf::Texture* SaveManager::getStage(int index) {
	index = std::clamp(index, 0, (int)stages.size()-1);
	return stages[index];
}