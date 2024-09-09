#ifndef GAME_SOUND_H
#define GAME_SOUND_H

#include <SFML/Audio.hpp>

#define SOUND_LIMIT 255

struct AudioManager {
	sf::Sound sounds[SOUND_LIMIT];
	sf::Sound music;

	int getFreeSound();
	sf::Sound* playSound(sf::SoundBuffer* buffer, bool variety = false);
	sf::Sound* playMusic(sf::SoundBuffer* buffer);

	void setVolume(float volume);
};

namespace g {
	extern AudioManager audio;
};

#endif