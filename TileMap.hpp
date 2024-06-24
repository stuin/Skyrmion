#pragma once

#include "Node.h"
#include "GridMaker.h"

#include <vector>
#include <stdexcept>

/*
 * Based off of sfml tutorial
 */

class TileMap : public Node {
private:
    const int tileX;
    const int tileY;
    Indexer *indexes;
    int offset = 0;

    //Graphical variables
    sf::VertexArray vertices;
    sf::Texture *tileset;
    sf::RenderTexture *buffer;

    /*virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        states.texture = tileset;
        target.draw(vertices, states);
    }*/

public:

    TileMap(sf::Texture *_tileset, int _tileX, int _tileY, Indexer *_indexes, Layer layer = 0, int _offset = 0)
     : Node(layer), tileX(_tileX), tileY(_tileY), indexes(_indexes), offset(_offset), tileset(_tileset) {

        //Set sizing
        unsigned int width = indexes->getSize().x;
        unsigned int height = indexes->getSize().y;
        setSize(sf::Vector2i(tileX * width, tileY * height));
        setOrigin(0, 0);

        //Set up buffer texture
        buffer = new sf::RenderTexture();
        if(!buffer->create(tileX * width, tileY * height))
            throw std::logic_error("Error creating TileMap buffer");
        setTexture(buffer->getTexture());

        // resize the vertex array to fit the level size
        vertices.setPrimitiveType(sf::Quads);
        vertices.resize(width * height * 4);

        //Load textures
        reload();
    }

    ~TileMap() {
    }

    void setOffset(int _offset) {
        offset = _offset;
    }

    int countTextures() {
        return (tileset->getSize().x / tileX) * (tileset->getSize().y / tileY);
    }

    void reload() {
        unsigned int width = indexes->getSize().x;
        unsigned int height = indexes->getSize().y;
        int *tiles = indexes->indexGrid();
        int numTextures = countTextures();

        // populate the vertex array, with one quad per tile
        for(unsigned int i = 0; i < width; ++i)
            for(unsigned int j = 0; j < height; ++j) {
                // get the current tile number
                int tileNumber = (tiles[i + j * width] % numTextures) + offset;
                int rotations = (tiles[i + j * width] / numTextures);

                // find its position in the tileset texture
                int tu = tileNumber % (tileset->getSize().x / tileX);
                int tv = tileNumber / (tileset->getSize().x / tileX);

                // get a pointer to the current tile's quad
                sf::Vertex* quad = &vertices[(i + j * width) * 4];

                if(tileNumber - offset != -1) {
                    // define its 4 texture coordinates
                    quad[(0 + rotations) % 4].texCoords = sf::Vector2f(tu * tileX, tv * tileY);
                    quad[(1 + rotations) % 4].texCoords = sf::Vector2f((tu + 1) * tileX, tv * tileY);
                    quad[(2 + rotations) % 4].texCoords = sf::Vector2f((tu + 1) * tileX, (tv + 1) * tileY);
                    quad[(3 + rotations) % 4].texCoords = sf::Vector2f(tu * tileX, (tv + 1) * tileY);

                    // define its 4 corners
                    quad[0].position = sf::Vector2f(i * tileX, j * tileY);
                    quad[1].position = sf::Vector2f((i + 1) * tileX, j * tileY);
                    quad[2].position = sf::Vector2f((i + 1) * tileX, (j + 1) * tileY);
                    quad[3].position = sf::Vector2f(i * tileX, (j + 1) * tileY);
                } else {
                    quad[0].position = sf::Vector2f(0, 0);
                    quad[1].position = sf::Vector2f(0, 0);
                    quad[2].position = sf::Vector2f(0, 0);
                    quad[3].position = sf::Vector2f(0, 0);
                }
            }
        UpdateList::scheduleReload(this);
    }

    void reloadBuffer() {
        //Draw to buffer
        buffer->clear(sf::Color::Transparent);
        buffer->draw(vertices, sf::RenderStates(tileset));
        buffer->display();
    }

    void setIndex(Indexer *indexes) {
        this->indexes = indexes;
        reload();
    }
};

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