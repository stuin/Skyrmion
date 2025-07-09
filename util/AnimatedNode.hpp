#pragma once

#include "../core/Node.h"

struct FrameTimer {
    int maxFrames = 0;
    int frame = 0;
    double maxTime = 0;
    double time = 0;

    FrameTimer(int _maxFrames, double _maxTime) {
        maxFrames = _maxFrames;
        maxTime = _maxTime;
        time = maxTime;
    }

    bool next(double delta) {
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
	AnimatedNode(sint texture, int _maxFrames, double _maxTime, Layer layer, Vector2i size, bool _horizontal=true) :
    Node(layer, size), timer(_maxFrames, _maxTime) {
		setTexture(texture);

        horizontal = _horizontal;

        //Get individual frame size
        frameSize = IO::getTextureSize(texture);
        if(horizontal)
		  frameSize.x /= _maxFrames;
        else
            frameSize.y /= _maxFrames;
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