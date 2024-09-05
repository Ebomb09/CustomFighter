#include "audio.h"

AudioManager g::audio = AudioManager();

int AudioManager::getFreeSound() {

	// Get the first free sound slot
	for(int i = 0; i < SOUND_LIMIT; i ++)
		if(sounds[i].getStatus() == sf::Sound::Status::Stopped)
			return i;

	return -1;
}

void AudioManager::playSound(sf::SoundBuffer* buffer) {
	int slot = getFreeSound();

	if(slot != -1) {
		sounds[slot].setBuffer(*buffer);
		sounds[slot].setPlayingOffset(sf::seconds(0));
		sounds[slot].play();
	}
}

void AudioManager::playMusic(std::string musicFile, bool loop) {
	music.openFromFile(musicFile);
	music.setPlayingOffset(sf::seconds(0));
	music.setLoop(loop);
	music.play();
}

void AudioManager::resumeMusic() {
	music.play();
}

void AudioManager::stopMusic() {
	music.stop();
}

void AudioManager::setVolume(float volume) {

	for(int i = 0; i < SOUND_LIMIT; i ++)
		sounds[i].setVolume(volume);

	music.setVolume(volume);
}