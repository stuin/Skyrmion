#include <vector>
#include "TileMap.hpp"

/*
 * Created by Stuart Irwin on 4/17/2019.
 * Builds animations out of tilemaps
 */

class AnimatedTileMap : public Node {
private:
	std::vector<TileMap *> tilemaps;
	int numTiles = 0;
	int maxFrames = 0;
	int frame = 0;
	double nextTime = 0;
	double delay = -1;
	int pauseAfter = 0;
	bool paused = false;

	//Draw selected tilemap
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		states.transform *= getTransform();
		target.draw(*tilemaps[frame], states);
    }

public:
	AnimatedTileMap(sf::Texture *tileset, int tileX, int tileY, Indexer *indexes, int frames, double delay, Layer layer = 0) : Node(layer) {
		int width = indexes->getSize().x;
        int height = indexes->getSize().y;
		setSize(sf::Vector2i(tileX * width, tileY * height));
		setOrigin(0, 0);

		this->maxFrames = frames;
		this->delay = delay;
		this->nextTime = delay;

		//Build each frame
		for(int i = 0; i < frames; i++) {
			//Load new tilemap
			TileMap *map = new TileMap(tileset, tileX, tileY, indexes, layer, i * numTiles);
		    tilemaps.push_back(map);

		    if(i == 0)
		    	this->numTiles = map->countTextures() / frames;
		}
    }

    AnimatedTileMap(sf::Vector2i size, double delay, Layer layer = 0) : Node(layer) {
    	this->numTiles = 0;
    	this->maxFrames = 0;
    	this->delay = delay;
    	this->nextTime = delay;
    	this->paused = true;

    	setSize(size);
    	setOrigin(0, 0);
    }

    void addFrame(TileMap *map) {
    	tilemaps.push_back(map);
    	maxFrames++;
    }

    TileMap *getFrame(int index) {
    	return tilemaps[index];
    }

    int getCurrentFrame() {
    	return frame;
    }

    void setCurrentFrame(int frame) {
    	this->frame = frame;
    }

    void setPaused(bool paused) {
    	this->paused = paused;
    }

    void setPauseAfter(int frame) {
    	this->pauseAfter = frame;
    }

	//Reload all tilemaps
	void reload() {
		for(int i = 0; i < maxFrames; i++)
			tilemaps[i]->reload(i * numTiles);
	}

	//Update timer
	void update(double time) {
		//Every half second
		if(!paused) {
			if((nextTime -= time) <= 0) {
				nextTime = delay;
				frame++;

				//Pause at certain frame
				if(pauseAfter > 0) {
					pauseAfter--;
					if(pauseAfter == 0)
						paused = true;
				}

				//Reset to start frame
				if(frame == maxFrames)
					frame = 0;
			}
		}
	}
};