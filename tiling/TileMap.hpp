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
    int tileset = 0;

    uint fullWidth = 0;
    uint fullHeight = 0;
    uint width = 0;
    uint height = 0;
    uint startX = 0;
    uint startY = 0;

public:

    TileMap(int _tileset, int _tileX, int _tileY, Indexer *_indexes, Layer layer = 0, int _offset = 0, Rect<uint> border=Rect<uint>())
     : Node(layer), tileX(_tileX), tileY(_tileY), indexes(_indexes), offset(_offset), tileset(_tileset) {

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
        setTexture(tileset);

        //std::cout << " " << startX << "," << startY << ", " << width << "," << height << "\n";
        //std::cout << toString(getGPosition()) << ":" << toString(getGScale()) <<  "\n";

        //Set up buffer texture
        //if(!buffer.create(tileX * width, tileY * height))
        //    throw std::logic_error("Error creating TileMap buffer");
        //setTexture(buffer.getTexture());

        // resize the vertex array to fit the level size
        //vertices.setPrimitiveType(sf::Quads);
        //vertices.resize(width * height * 4);
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
        return (UpdateList::getTextureSize(tileset).x / tileX) * (UpdateList::getTextureSize(tileset).y / tileY);
    }

    void reload() {
        int numTextures = countTextures();
        int usedRects = 0;

        // populate the vertex array, with one quad per tile
        for(unsigned int i = 0; i < width; ++i) {
            for(unsigned int j = 0; j < height; ++j) {
                // get the current tile number
                int tileValue = indexes->getTile(Vector2f(i + startX, j + startY));
                int tileNumber = (tileValue % numTextures) + offset;
                int rotations = (tileValue / numTextures) % 4;
                int fliph = rotations / 4 % 2;
                int flipv = rotations / 8;

                // find its position in the tileset texture
                int tu = tileNumber % (UpdateList::getTextureSize(tileset).x / tileX);
                int tv = tileNumber / (UpdateList::getTextureSize(tileset).x / tileX);

                // get a pointer to the current tile's quad
                TextureRect quad = (*getTextureRects())[usedRects];
                //sf::Vertex* quad = &vertices[(i + j * width) * 4];

                if(tileNumber - offset != -1) {
                    quad.tx = (tu+fliph) * tileX;
                    quad.ty = (tv+flipv) * tileY;
                    quad.width = fliph ? -tileX : tileX;
                    quad.height = flipv ? -tileY : tileY;
                    quad.px = i * tileX;
                    quad.py = j * tileY;
                    quad.rotation = PIO2*rotations;
                    setTextureRect(quad, usedRects++);
                }

                /*if(tileNumber - offset != -1) {
                    // define its 4 texture coordinates
                    quad[(0 + rotations + fliph - flipv) % 4].texCoords = Vector2f(tu * tileX, tv * tileY);
                    quad[(1 + rotations - fliph + flipv) % 4].texCoords = Vector2f((tu + 1) * tileX, tv * tileY);
                    quad[(2 + rotations + fliph - flipv) % 4].texCoords = Vector2f((tu + 1) * tileX, (tv + 1) * tileY);
                    quad[(3 + rotations - fliph + flipv) % 4].texCoords = Vector2f(tu * tileX, (tv + 1) * tileY);

                    // define its 4 corners
                    quad[0].position = Vector2f((i) * tileX, (j) * tileY);
                    quad[1].position = Vector2f((i + 1) * tileX, (j) * tileY);
                    quad[2].position = Vector2f((i + 1) * tileX, (j + 1) * tileY);
                    quad[3].position = Vector2f((i) * tileX, (j + 1) * tileY);
                } else {
                    quad[0].position = Vector2f(0, 0);
                    quad[1].position = Vector2f(0, 0);
                    quad[2].position = Vector2f(0, 0);
                    quad[3].position = Vector2f(0, 0);
                }*/
            }
        }
        getTextureRects()->resize(usedRects);
        UpdateList::scheduleReload(this);
    }

    void reloadBuffer() {
        //Draw to buffer
        //buffer.clear(sf::Color::Transparent);
        //buffer.draw(vertices, sf::RenderStates(tileset));
        //buffer.display();
    }

    void setIndexer(Indexer *indexes) {
        this->indexes = indexes;
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
        setTexture(tileset);

        this->maxFrames = frames;
        this->delay = delay;
        this->nextTime = delay;

        //Build each frame
        for(int i = 0; i < frames; i++) {
            //Load new tilemap
            TileMap *map = new TileMap(tileset, tileX, tileY, indexes, layer, i * numTiles);
            map->setParent(this);
            tilemaps.push_back(map);

            if(i == 0)
                this->numTiles = map->countTextures() / frames;
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
            }
        }
    }

    std::vector<TextureRect> *getTextureRects() {
        return tilemaps[frame]->getTextureRects();
    }

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
        setHidden(true);
        setTexture(tileset);

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
        for(TileMap *map : tilemaps)
            nodes.push_back(map);
        return nodes;
    }
};