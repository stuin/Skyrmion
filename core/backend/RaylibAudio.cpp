#include "../../include/raylib/src/raylib.h"//

#include "../AudioList.h"
#include "../../tiling/NoiseIndexer.hpp"

std::vector<AudioChannel> channels;
std::vector<AudioEvent> events;
std::vector<Sound> eventSounds;
int audioSeed = 0;

void AudioList::initAudio(std::vector<std::string> _channels, std::vector<std::string> files) {
	InitAudioDevice();
	createChannel(0, "Default");
	audioSeed = intNoise(time(NULL));

	for(std::string channel : _channels)
		channels.emplace_back(channel);

	for(std::string file : files) {
		events.emplace_back(file);
		if(file[0] == '_') {
			eventSounds.emplace_back();
		} else {
			eventSounds.push_back(LoadSound(file.c_str()));
			if(IsSoundValid(eventSounds.back()))
				events.back().valid = true;
		}
	}
}

void AudioList::createChannel(sint c, std::string name, int volume, bool stream) {
	if(c <= channels.size())
		channels.resize(c+1);
	channels[c] = AudioChannel(name, volume, stream);
}

void AudioList::assignEvent(sint channel, sint e, int variants) {
	if(e+variants <= events.size()) {
		events.resize(e+1+variants);
		eventSounds.resize(e+1+variants);
	}
	events[e].channel = channel;
	events[e].variants = variants;

	for(int v = 1; v <= variants; v++)
		assignEvent(channel, e+v, -v);
}

void AudioList::assignEvents(sint channel, sint min, sint max) {
	for(int i = min; i <= max; i++)
		assignEvent(i, channel);
}

AudioChannel AudioList::getChannel(sint c) {
	return channels[c];
}
AudioEvent AudioList::getEvent(sint e) {
	return events[e];
}

int AudioList::getAudioSeed() {
	int seed = audioSeed;
	audioSeed = intNoise(audioSeed);
	return seed;
}

void AudioList::playEvent(sint e, Node *source) {
	AudioEvent event = events[e];
	IO::queueEvent(EVENT_SOUND, true, e);

	if(event.variants > 0) {
		e = e+intNoise(audioSeed, event.variants-1)+1;
		event = events[e];
		audioSeed = intNoise(audioSeed);
	}

	if(event.valid) {
		SetSoundVolume(eventSounds[e], channels[event.channel].volume/100.0f);
		PlaySound(eventSounds[e]);
	}

}

bool AudioList::isPlaying(sint e) {
	if(IsSoundPlaying(eventSounds[e]))
		return true;

	AudioEvent event = events[e];
	for(int v = 1; v < event.variants+1; v++)
		if(IsSoundPlaying(eventSounds[e+v]))
			return true;
	return false;
}

void AudioList::setVolume(sint c, int volume) {
	channels[c].volume = volume;
}

void AudioList::setMasterVolume(int volume) {
	SetMasterVolume(volume/100.0);
}

//Background music streaming
Music backgroundMusic;
void AudioList::musicStream(std::string filename, int volume) {
	backgroundMusic = LoadMusicStream(filename.c_str());
	SetMusicVolume(backgroundMusic, volume/100.0);
	PlayMusicStream(backgroundMusic);
}

void AudioList::processAudio() {
	//Play streaming audio
	if(IsAudioDeviceReady() && IsMusicStreamPlaying(backgroundMusic))
		UpdateMusicStream(backgroundMusic);
}

void AudioList::cleanupAudio() {

}