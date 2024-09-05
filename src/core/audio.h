#ifndef GAME_SOUND_H
#define GAME_SOUND_H

#include <SFML/Audio.hpp>
#include <string>

#define SOUND_LIMIT 256

struct AudioManager {
	sf::Sound sounds[SOUND_LIMIT];
	sf::Music music;

	int getFreeSound();
	void playSound(sf::SoundBuffer* buffer);

	void playMusic(std::string musicFile, bool loop = false);
	void resumeMusic();
	void stopMusic();

	void setVolume(float volume);
};

namespace g {
	extern AudioManager audio;
};

#endif