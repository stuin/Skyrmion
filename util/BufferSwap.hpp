#include "AnimatedNode.hpp"

#include "../core/UpdateList.h"

class BufferSwap : public Node {
    FrameTimer timer;
    bool paused = false;

	sint firstBuffer;
	sint secondBuffer;
	bool second = false;
public:
	BufferSwap(int layer, sint _firstBuffer, sint _secondBuffer, Vector2i size, double _maxTime) :
    Node(layer, RENDER_TEXTURE_SINGLE, size), timer(2, _maxTime) {

    	firstBuffer = _firstBuffer;
		secondBuffer = _secondBuffer;

    	setOrigin(0,0);
    	setTexture(firstBuffer);

    	//setupBuffer(firstBuffer);
    	//setTexture(secondBuffer);
		//UpdateList::createBuffer(BufferData(rIndex, this, _color));
	}

	void setPaused(bool _paused=true) {
		paused = _paused;
	}

	FrameTimer &getTimer() {
    	return timer;
    }

    void update(double time) {
        updateSwap(time);
    }

    //Update timer
    void updateSwap(double time) {
    	if(!paused && timer.next(time)) {
    		second = !second;
            if(second) {
            	UpdateList::scheduleBufferRefresh(secondBuffer);
            	setTexture(secondBuffer);
            } else {
            	UpdateList::scheduleBufferRefresh(firstBuffer);
            	setTexture(firstBuffer);
            }
        }
    }
};