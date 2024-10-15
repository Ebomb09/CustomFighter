#ifndef GAME_CLOTHING_H
#define GAME_CLOTHING_H

#include <filesystem>
#include <SFML/Graphics.hpp>

namespace ClothingSpace {

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

	const std::string String[] {
		"TorsoFront",
		"TorsoBack",
		"Neck",
		"UpperArm",
		"ForeArm",
		"Pelvis",
		"Thigh",
		"Calf",
		"HandFront",
		"HandBack",
		"Foot",
		"Head",
		"Total"
	};
};

struct Clothing {
	std::string name;
	sf::Color blend;

	union {
		sf::Texture* part[ClothingSpace::Total];

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