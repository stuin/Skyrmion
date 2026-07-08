#pragma once

#include "../core/Node.h"
#include "../tiling/GridMaker.h"

#include <vector>
#include <stdexcept>

/*
 * Featureful TileMap renderer
 * Originally based off of sfml tutorial
 */

class TileMap : public Node {
private:
    Vector2i tileSize;

    Indexer *indexes;
    uint gridUpdates = 0;

    int offset = 0;
    Vector2i overlap;
    bool hexRows;

    Vector2i fullSize;
    Vector2i rectSize;
    Vector2i rectPos;

public:

    TileMap(sint _tileset, int _tileX, int _tileY, Indexer *_indexes, int layer=0, int _offset=0, bool _hexRows=false, Rect<uint> border=Rect<uint>())
     : Node(layer, RENDER_TEXTURE_ARRAY), tileSize(_tileX, _tileY), indexes(_indexes), offset(_offset), hexRows(_hexRows) {

        //Set sizing
        fullSize = rectSize = indexes->getSize() * indexes->getScale();
        rectPos = border.pos();
        if(border.width != 0)
            rectSize.x = border.width;
        if(border.height != 0)
            rectSize.y = border.height;
        if(rectSize.x + rectPos.x > fullSize.x)
            rectSize.x = fullSize.x - rectPos.x;
        if(rectSize.y + rectPos.y > fullSize.y)
            rectSize.y = fullSize.y - rectPos.y;

        if(_hexRows)
            overlap = Vector2i(0, _tileY/4);

        setSize((tileSize - overlap) * rectSize);
        setOrigin(0, 0);
        setPosition((tileSize - overlap) * rectPos);

        setTexture(_tileset);
        setupBuffer(0, COLOR_EMPTY);

        //std::cout << " " << startX << "," << startY << ", " << width << "," << height << "\n";
        //std::cout << toString(getGPosition()) << ":" << toString(getGScale()) <<  "\n";

        getTextureRects()->reserve(rectSize.x * rectSize.y);

        //Load textures
        reload();
    }

    ~TileMap() {
    }

    void setOffset(int _offset) {
        offset = _offset;
    }

    int countTextures() {
        return (getTextureSize().x / tileSize.x) * (getTextureSize().y / tileSize.y);
    }

    void reload() {
        int numTextures = countTextures();
        int usedRects = 0;
        bool hasBuffer = true;
        int rotationCount = (hexRows) ? 6 : 4;

        // populate the vertex array, with one quad per tile
        for(int j = 0; j < rectSize.y; ++j) {
            for(int i = 0; i < rectSize.x; ++i) {
                // get the current tile number
                int tileValue = indexes->getTile(Vector2f(i + rectPos.x, hasBuffer ? fullSize.y - (j + rectPos.y + 1) : j + rectPos.y));
                int tileNumber = (tileValue % numTextures) + offset;
                int rotations = (tileValue / numTextures);
                int fliph = rotations / rotationCount % 2;
                int flipv = rotations / (rotationCount * 2);

                if(hasBuffer)
                    flipv = (flipv == 0) ? 1 : 0;

                // find its position in the tileset texture
                int tu = tileNumber % (getTextureSize().x / tileSize.x);
                int tv = tileNumber / (getTextureSize().x / tileSize.x);

                int xOffset = 0;
                if(hexRows && (j + rectPos.y) % 2 == 1)
                    xOffset = tileSize.x / 2;

                if(tileNumber - offset != -1) {
                    TextureRect quad;
                    quad.px = i * (tileSize.x - overlap.x) + xOffset;
                    quad.py = j * (tileSize.y - overlap.y);
                    quad.pwidth = fliph ? -tileSize.x : tileSize.x;
                    quad.pheight = flipv ? -tileSize.y : tileSize.y;
                    quad.tx = tu * tileSize.x;
                    quad.ty = tv * tileSize.y;
                    quad.twidth = fliph ? -tileSize.x : tileSize.x;
                    quad.theight = flipv ? -tileSize.y : tileSize.y;
                    quad.rotation = (360/rotationCount)*(rotations % rotationCount);
                    setTextureRect(quad, usedRects++);
                }
            }
        }
        gridUpdates = indexes->getUpdateCount();
        getTextureRects()->resize(usedRects);

        if(hasBuffer)
            scheduleBufferRefresh();
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
    AnimatedTileMap(int tileset, int tileX, int tileY, Indexer *indexes, int frames, double delay, int layer = 0)
     : Node(layer, RENDER_TEXTURE_SINGLE) {
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
                setTexture(map->getRenderComponent(false)->getTexture());
            }
        }
    }

    AnimatedTileMap(Vector2i size, double delay, int layer = 0) : Node(layer) {
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
                setTexture(tilemaps[frame]->getRenderComponent(false)->getTexture());
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
    LargeTileMap(int tileset, int tileX, int tileY, Indexer *indexes, int layer) : Node(layer, RENDER_NONE) {
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
                TileMap *map = new TileMap(tileset, tileX, tileY, indexes, layer, 0, false, border);
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