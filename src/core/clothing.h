#ifndef GAME_CLOTHING_H
#define GAME_CLOTHING_H

#include <SFML/Graphics.hpp>

struct Clothing {
	sf::Texture *torsoFront		= NULL,
				*torsoBack		= NULL,
				*neck 			= NULL,
				*upperArm 		= NULL,
				*foreArm 		= NULL,
				*pelvis 		= NULL,
				*thigh 			= NULL,
				*calf 			= NULL,
				*hand			= NULL,
				*foot			= NULL,
				*head			= NULL;
};

#endif