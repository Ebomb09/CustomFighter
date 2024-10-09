#include "audio.h"

#include <iostream>

AudioManager g::audio = AudioManager();

int AudioManager::getFreeSound() {

	// Get the first free sound slot
	for(int i = 0; i < SOUND_LIMIT; i ++)
		if(sounds[i].getStatus() == sf::Sound::Status::Stopped)
			return i;

	return -1;
}

sf::Sound* AudioManager::playSound(sf::SoundBuffer* buffer, bool variety) {

	if(stackEnable > 0)
		return NULL;

	if(!buffer) {
		std::cerr << "Invalid sound buffer provided\n";
		return NULL;
	}

	int slot = getFreeSound();

	if(slot != -1) {
		sounds[slot].setBuffer(*buffer);
		sounds[slot].setPlayingOffset(sf::seconds(0));
		sounds[slot].setPitch(1.f);
		sounds[slot].setLoop(false);

		if(variety)
			sounds[slot].setPitch(1.f + (rand() % 100 + 1 - 50) / 200.f);
		
		sounds[slot].play();
		return &sounds[slot];
	}

	std::cerr << "Too many sounds playing\n";
	return NULL;
}

sf::Sound* AudioManager::playMusic(sf::SoundBuffer* buffer) {

	if(stackEnable > 0)
		return NULL;

	if(!buffer) {
		std::cerr << "Invalid sound buffer provided\n";
		return NULL;
	}

	music.setBuffer(*buffer);
	music.setPlayingOffset(sf::seconds(0));
	music.play();
	return &music;
}

void AudioManager::setVolume(float volume) {

	for(int i = 0; i < SOUND_LIMIT; i ++)
		sounds[i].setVolume(volume);

	music.setVolume(volume);
}

void AudioManager::disable() {
	stackEnable ++;
}

void AudioManager::enable() {

	if(stackEnable > 0)
		stackEnable --;
}

bool AudioManager::isEnabled() {
	return stackEnable == 0;
}