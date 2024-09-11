#ifndef GAME_CLOTHING_H
#define GAME_CLOTHING_H

#include <filesystem>
#include <SFML/Graphics.hpp>

struct Clothing {
	std::string name;
	sf::Color blend;

	enum {
		TorsoFront,
		TorsoBack,
		Neck,
		UpperArm,
		ForeArm,
		Pelvis,
		Thigh,
		Calf,
		HandFront,
		HandBack,
		Foot,
		Head,
		Total
	};

	union {
		sf::Texture* part[Total];

		struct{
			sf::Texture *torsoFront,
						*torsoBack,
						*neck,
						*upperArm,
						*foreArm,
						*pelvis,
						*thigh,
						*calf,
						*handFront,
						*handBack,
						*foot,
						*head;			
		};
	};

	Clothing();

	bool loadFromFile(std::filesystem::path path);
};

#endif