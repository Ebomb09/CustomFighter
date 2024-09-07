#ifndef GAME_CLOTHING_H
#define GAME_CLOTHING_H

#include <SFML/Graphics.hpp>

struct Clothing {
	std::string name = "";
	sf::Color blend = sf::Color(255, 255, 255);

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
			sf::Texture *torsoFront		= NULL,
						*torsoBack		= NULL,
						*neck 			= NULL,
						*upperArm 		= NULL,
						*foreArm 		= NULL,
						*pelvis 		= NULL,
						*thigh 			= NULL,
						*calf 			= NULL,
						*handFront		= NULL,
						*handBack		= NULL,
						*foot			= NULL,
						*head			= NULL;			
		};
	};
};

#endif