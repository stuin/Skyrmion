#pragma once

#include "../core/Node.h"

struct FrameTimer {
    int maxFrames = 0;
    int frame = 0;
    float maxTime = 0;
    float time = 0;

    FrameTimer(int _maxFrames, float _maxTime) {
        maxFrames = _maxFrames;
        maxTime = _maxTime;
        time = maxTime;
    }

    bool next(float delta) {
        if((time -= delta) <= 0 || time > maxTime) {
            time = maxTime;
            frame++;

            //Reset to start frame
            if(frame >= maxFrames)
                frame = 0;
            return true;
        }
        return false;
    }

    void print() {
        std::cout << frame << "/" << maxFrames << " " << time << "/" << maxTime << "\n";
    }
};

//Handles simple animations with horizontal spritesheets
class AnimatedNode : public Node {
    FrameTimer timer;
    bool paused = false;

	Vector2i frameSize;
    bool horizontal;

public:
	AnimatedNode(sint texture, int _maxFrames, double _maxTime, int layer, Vector2i size, bool _horizontal=true) :
    Node(layer, RENDER_TEXTURE_RECT, size), timer(_maxFrames, _maxTime) {
		setTexture(texture);

        horizontal = _horizontal;

        //Get individual frame size
        frameSize = getTextureSize();
        if(horizontal)
		  frameSize.x /= _maxFrames;
        else
            frameSize.y /= _maxFrames;
	}

    void setPaused(bool _paused=true) {
        paused = _paused;
    }

    FrameTimer &getTimer() {
        return timer;
    }

    void update(double time) {
        updateAnimation(time);
    }

    //Update timer
    void updateAnimation(double time) {
    	if(!paused && timer.next(time)) {
            if(horizontal)
                setTextureIntRect(IntRect(frameSize.x * timer.frame, 0, frameSize.x, frameSize.y));
            else
                setTextureIntRect(IntRect(0, frameSize.y * timer.frame, frameSize.x, frameSize.y));
        }
    }
};