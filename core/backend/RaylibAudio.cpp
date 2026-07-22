#include "../../include/raylib/src/raylib.h"//

#include "../AudioList.h"

//Audio systems
void AudioList::setVolume(int volume) {
	if(!IsAudioDeviceReady())
		InitAudioDevice();

	SetMasterVolume(volume/100.0);
}

//Background music
Music backgroundMusic;
void AudioList::musicStream(std::string filename, int volume) {
	if(!IsAudioDeviceReady())
		InitAudioDevice();

	backgroundMusic = LoadMusicStream(filename.c_str());
	SetMusicVolume(backgroundMusic, volume/100.0);
	PlayMusicStream(backgroundMusic);
}

void AudioList::processAudio() {
	//Play audio
	if(IsAudioDeviceReady() && IsMusicStreamPlaying(backgroundMusic))
		UpdateMusicStream(backgroundMusic);
}

void AudioList::initAudio() {

}

void AudioList::cleanupAudio() {

}