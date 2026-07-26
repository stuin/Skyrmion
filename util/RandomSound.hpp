#include "../core/AudioList.h"
#include "../tiling/NoiseIndexer.hpp"

class RandomSound : public UNode {
	sint sound;
	float minDelay;
	float maxDelay;

	float currentDelay = 0;

public:
	bool paused = false;

	RandomSound(sint _sound, float _minDelay, float _maxDelay, int layer=0) : UNode(layer) {
		sound = _sound;
		minDelay = _minDelay;
		maxDelay = _maxDelay;
	}

	void update(double time) {
		if(!paused && !AudioList::isPlaying(sound)) {
			currentDelay += time;
			if(currentDelay >= maxDelay) {
				AudioList::playEvent(sound);
				currentDelay = 0;
			} else if(currentDelay >= minDelay) {
				float diff = maxDelay-minDelay;
				float current = currentDelay-minDelay;
				if(stableNoise(AudioList::getAudioSeed()) > current/diff) {
					AudioList::playEvent(sound);
					currentDelay = 0;
				}
			}
		}
	}
};