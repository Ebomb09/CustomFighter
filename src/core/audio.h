#ifndef GAME_SOUND_H
#define GAME_SOUND_H

#include <SFML/Audio.hpp>

#define SOUND_LIMIT 255

class AudioManager {
	sf::Sound sounds[SOUND_LIMIT];
	sf::Sound music;

	int stackEnable = 0;

	int getFreeSound();

public:
	sf::Sound* playSound(sf::SoundBuffer* buffer, bool variety = false);
	sf::Sound* playMusic(sf::SoundBuffer* buffer);

	void setVolume(float volume);
	void setMusicVolume(float volume);
	void setSoundVolume(float volume);

	void disable();
	void enable();

	bool isEnabled();
};

namespace g {
	extern AudioManager audio;
};

#endif