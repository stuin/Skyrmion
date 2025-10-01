#pragma once

#include "../core/Node.h"
#include "../core/UpdateList.h"
#include "../tiling/GridMaker.h"

#include <vector>
#include <stdexcept>

/*
 * Featureful TileMap renderer
 * Originally based off of sfml tutorial
 */

class TileMap : public Node {
private:
    const int tileX;
    const int tileY;
    Indexer *indexes;
    int offset = 0;
    uint gridUpdates = 0;
    sint tileset;
    sint buffer;

    uint fullWidth = 0;
    uint fullHeight = 0;
    uint width = 0;
    uint height = 0;
    uint startX = 0;
    uint startY = 0;

public:
    TileMap(sint _tileset, int _tileX, int _tileY, Indexer *_indexes, Layer layer=0, int _offset=0, Rect<uint> border=Rect<uint>())
     : Node(layer), tileX(_tileX), tileY(_tileY), indexes(_indexes), offset(_offset) {

        //Set sizing
        fullWidth = width = indexes->getSize().x;
        fullHeight = height = indexes->getSize().y;
        startX = border.left;
        startY = border.top;
        if(border.width != 0)
            width = border.width;
        if(border.height != 0)
            height = border.height;
        if(width + startX > fullWidth)
            width = fullWidth - startX;
        if(height + startY > fullHeight)
            height = fullHeight - startY;

        setSize(Vector2i(tileX * width, tileY * height));
        setOrigin(0, 0);
        setPosition(startX * tileX, startY * tileY);

        tileset = _tileset;
        setTexture(tileset);
        buffer = UpdateList::createBuffer(BufferData(UpdateList::getResourceCount(), this, COLOR_EMPTY));
        setTexture(buffer);

        //std::cout << " " << startX << "," << startY << ", " << width << "," << height << "\n";
        //std::cout << toString(getGPosition()) << ":" << toString(getGScale()) <<  "\n";

        getTextureRects()->reserve(width * height);

        //Load textures
        reload();
    }

    ~TileMap() {
    }

    void setOffset(int _offset) {
        offset = _offset;
    }

    int countTextures() {
        return (IO::getTextureSize(tileset).x / tileX) * (IO::getTextureSize(tileset).y / tileY);
    }

    void reload() {
        int numTextures = countTextures();
        int usedRects = 0;
        bool hasBuffer = true;

        // populate the vertex array, with one quad per tile
        for(unsigned int j = 0; j < height; ++j) {
            for(unsigned int i = 0; i < width; ++i) {
                // get the current tile number
                int tileValue = indexes->getTile(Vector2f(i + startX, hasBuffer ? fullHeight - (j + startY + 1) : j + startY));
                int tileNumber = (tileValue % numTextures) + offset;
                int rotations = (tileValue / numTextures);
                int fliph = rotations / 4 % 2;
                int flipv = rotations / 8;

                if(hasBuffer)
                    flipv = (flipv == 0) ? 1 : 0;

                // find its position in the tileset texture
                int tu = tileNumber % (IO::getTextureSize(tileset).x / tileX);
                int tv = tileNumber / (IO::getTextureSize(tileset).x / tileX);

                if(tileNumber - offset != -1) {
                    TextureRect quad;
                    quad.px = i * tileX;
                    quad.py = j * tileY;
                    quad.pwidth = fliph ? -tileX : tileX;
                    quad.pheight = flipv ? -tileY : tileY;
                    quad.tx = tu * tileX;
                    quad.ty = tv * tileY;
                    quad.twidth = fliph ? -tileX : tileX;
                    quad.theight = flipv ? -tileY : tileY;
                    quad.rotation = 90*(rotations % 4);
                    setTextureRect(quad, usedRects++);
                }
            }
        }
        gridUpdates = indexes->getUpdateCount();
        getTextureRects()->resize(usedRects);

        if(hasBuffer)
            UpdateList::scheduleReload(buffer);
    }

    void setIndexer(Indexer *indexes) {
        this->indexes = indexes;
        reload();
    }

    void update(double time) {
        if(this->indexes->getUpdateCount() != gridUpdates)
            reload();
    }
};

//Swap between multiple tilemaps with offset textures
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

public:
    AnimatedTileMap(int tileset, int tileX, int tileY, Indexer *indexes, int frames, double delay, Layer layer = 0) : Node(layer) {
        int width = indexes->getSize().x;
        int height = indexes->getSize().y;
        setSize(Vector2i(tileX * width, tileY * height));
        setOrigin(0, 0);

        this->maxFrames = frames;
        this->delay = delay;
        this->nextTime = delay;

        //Build each frame
        for(int i = 0; i < frames; i++) {
            //Load new tilemap
            TileMap *map = new TileMap(tileset, tileX, tileY, indexes, layer, i * numTiles);
            map->setParent(this);
            tilemaps.push_back(map);

            if(i == 0) {
                this->numTiles = map->countTextures() / frames;
                setTexture(map->getTexture());
            }
        }
    }

    AnimatedTileMap(Vector2i size, double delay, Layer layer = 0) : Node(layer) {
        this->numTiles = 0;
        this->maxFrames = 0;
        this->delay = delay;
        this->nextTime = delay;
        this->paused = true;

        setSize(size);
        setOrigin(0, 0);
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
                setTexture(tilemaps[frame]->getTexture());
            }
        }
        for(TileMap *map : tilemaps)
            map->update(time);
    }

    //std::vector<TextureRect> *getTextureRects() {
    //    return tilemaps[frame]->getTextureRects();
    //}

    void addFrame(TileMap *map) {
        map->setOffset(tilemaps.size() * numTiles);
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
            tilemaps[i]->reload();
    }

    std::vector<Node *> getNodes() {
        std::vector<Node *> nodes;
        nodes.push_back(this);
        for(TileMap *map : tilemaps)
            nodes.push_back(map);
        return nodes;
    }
};

//Split up tilemaps that are too large for one buffer
class LargeTileMap : public Node {
private:
    std::vector<TileMap *> tilemaps;

    uint fullWidth;
    uint fullHeight;
    uint sectionWidth;
    uint sectionHeight;
    uint countX;
    uint countY;

public:
    LargeTileMap(int tileset, int tileX, int tileY, Indexer *indexes, Layer layer) : Node(layer) {
        fullWidth = indexes->getSize().x;
        fullHeight = indexes->getSize().y;
        setSize(Vector2i(tileX * fullWidth, tileY * fullHeight));
        setOrigin(0, 0);

        countX = std::ceil(fullWidth / (16000.0 / tileX));
        countY = std::ceil(fullHeight / (16000.0 / tileY));
        sectionWidth = fullWidth / countX;
        sectionHeight = fullHeight / countY;

        //std::cout << countX << "," << countY << ": " << fullWidth << "," << fullHeight << "\n";
        //std::cout << toString(getGPosition()) << ":" << toString(getScale()) <<  "\n";

        //Build each frame
        for(uint x = 0; x < countX; x++) {
            for(uint y = 0; y < countY; y++) {
                //Add new tilemap
                Rect<uint> border(x * sectionWidth, y * sectionHeight, sectionWidth, sectionHeight);
                TileMap *map = new TileMap(tileset, tileX, tileY, indexes, layer, 0, border);
                map->setParent(this);
                tilemaps.push_back(map);
            }
        }
    }

    //Reload all tilemaps
    void reload() {
        for(TileMap *map : tilemaps)
            map->reload();
    }

    std::vector<Node *> getNodes() {
        std::vector<Node *> nodes;
        //nodes.push_back(this);
        for(TileMap *map : tilemaps)
            nodes.push_back(map);
        return nodes;
    }
};