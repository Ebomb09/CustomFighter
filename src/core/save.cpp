#include "save.h"

SaveManager g::save = SaveManager("data");

SaveManager::~SaveManager() {

	for(auto& it : textures)
		delete it.second;

	for(auto& it : clothes)
		delete it.second;

	for(auto& it : animations)
		delete it.second;	
}

SaveManager::SaveManager(std::filesystem::path path) {

	// Load all clothing
	for(auto& entry : std::filesystem::directory_iterator(path/"clothing"))
		getClothing(entry);

	for(auto& entry : std::filesystem::directory_iterator(path/"moves"))
		getAnimation(entry);	
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

Clothing* SaveManager::getClothing(std::filesystem::path path) {
	Clothing* ptr = NULL;

	if(clothes.find(path.string()) == clothes.end()) {
		ptr = new Clothing;
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
		clothes[path.stem().string()] = ptr;
	
	}else {
		ptr = clothes.at(path.string());
	}
	return ptr;
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