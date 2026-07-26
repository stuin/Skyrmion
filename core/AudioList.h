#pragma once

#include "Node.h"
#include "../input/Settings.h"

struct AudioChannel {
	std::string name = "UNKNOWN";
	int volume = 100;
	bool stream = false;

	AudioChannel() {}
	AudioChannel(std::string _name, int _volume=100, bool _stream=false) {
		name = _name;
		volume = _volume;
		stream = _stream;
	}
};

struct AudioEvent {
	std::string file = "UNKNOWN";
	bool valid = false;
	int channel = 0;
	int variants = 0;

	AudioEvent() {}
	AudioEvent(std::string _file) {
		file = _file;
	}
};

class AudioList {
public:
	//Audio setup
	static void initAudio(std::vector<std::string> channels, std::vector<std::string> files);
	static void createChannel(sint c, std::string name, int volume=100, bool stream=false);
	static void assignEvent(sint channel, sint e, int variants=0);
	static void assignEvents(sint channel, sint min, sint max);

	static AudioChannel getChannel(sint c);
	static AudioEvent getEvent(sint e);
	static int getAudioSeed();

	//Controls
	static void playEvent(sint event, Node *source=NULL);
	static bool isPlaying(sint event);
	static void setVolume(sint channel, int volume);
	static void setMasterVolume(int volume);

	//Seperate music stream player
	static void musicStream(std::string filename, int volume=100);

	//Called by UpdateList
	static void processAudio();
	static void cleanupAudio();
	static void stream_cb(float* bufferOut, int num_frames, int num_channels);
};