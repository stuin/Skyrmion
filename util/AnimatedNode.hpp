#include "../core/UpdateList.h"

//Handles simple animations with horizontal spritesheets
class AnimatedNode : public Node {
	Vector2i frameSize;
    bool horizontal;
    int maxFrames = 0;
    int frame = 0;
    double nextTime = 0;
    double delay = -1;
    bool paused = false;
public:
	AnimatedNode(sint texture, int _maxFrames, double _delay, Layer layer, Vector2i size, bool _horizontal=true) : Node(layer, size) {
		setTexture(texture);

        horizontal = _horizontal;
		maxFrames = _maxFrames;
		delay = _delay;

        //Get individual frame size
        frameSize = UpdateList::getTextureSize(texture);
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
    	if(!paused) {
            if((nextTime -= time) <= 0) {
                nextTime = delay;
                frame++;

                //Reset to start frame
                if(frame == maxFrames)
                    frame = 0;

                if(horizontal)
                    setTextureRect(IntRect(frameSize.x * frame, 0, frameSize.x, frameSize.y));
                else
                    setTextureRect(IntRect(0, frameSize.y * frame, frameSize.x, frameSize.y));
            }
        }
    }
};