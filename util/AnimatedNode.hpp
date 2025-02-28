#include "../core/UpdateList.h"

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
            timer.next(time);

            if(horizontal)
                setTextureRect(IntRect(frameSize.x * timer.frame, 0, frameSize.x, frameSize.y));
            else
                setTextureRect(IntRect(0, frameSize.y * timer.frame, frameSize.x, frameSize.y));
        }
    }
};