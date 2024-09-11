#include "clothing.h"
#include "save.h"

Clothing::Clothing() {
	name = "";
	blend = sf::Color::White;

	for(int i = 0; i < Total; i ++)
		part[i] = NULL;
}

bool Clothing::loadFromFile(std::filesystem::path path) {
	name		= path.stem().string();

	torsoFront	= g::save.getTexture(path/"torsoFront.png");
	torsoBack	= g::save.getTexture(path/"torsoBack.png");
	neck 		= g::save.getTexture(path/"neck.png");
	upperArm 	= g::save.getTexture(path/"upperArm.png");
	foreArm 	= g::save.getTexture(path/"foreArm.png");
	pelvis 		= g::save.getTexture(path/"pelvis.png");
	thigh 		= g::save.getTexture(path/"thigh.png");
	calf 		= g::save.getTexture(path/"calf.png");
	handFront 	= g::save.getTexture(path/"handFront.png");
	handBack 	= g::save.getTexture(path/"handBack.png");					
	foot 		= g::save.getTexture(path/"foot.png");
	head 		= g::save.getTexture(path/"head.png");

	// Check if any part was loaded
	for(int i = 0; i < Clothing::Total; i ++)
		if(part[i])
			return true;

	return false;
}