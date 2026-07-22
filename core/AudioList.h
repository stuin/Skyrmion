#include "Node.h"

class AudioList {
public:
	//Called by UpdateList
	static void initAudio();
	static void processAudio();
	static void cleanupAudio();
	static void stream_cb(float* bufferOut, int num_frames, int num_channels);

	//Audio controls
	static void setVolume(int volume);
	static void musicStream(std::string filename, int volume=100);
};