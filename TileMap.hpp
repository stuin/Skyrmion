#pragma once

#include "Node.h"
#include "GridMaker.h"

#include <stdexcept>

/*
 * Based off of sfml tutorial
 */

class TileMap : public Node {
private:
    const int tileX;
    const int tileY;
    Indexer *indexes;

    //Graphical variables
    sf::VertexArray vertices;
    sf::Texture *tileset;
    sf::RenderTexture *buffer;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        states.texture = tileset;
        target.draw(vertices, states);
    }

public:

    TileMap(sf::Texture *_tileset, int _tileX, int _tileY, Indexer *_indexes, Layer layer = 0, int offset = 0)
     : Node(layer), tileX(_tileX), tileY(_tileY), indexes(_indexes), tileset(_tileset) {

        //Set sizing
        unsigned int width = indexes->getSize().x;
        unsigned int height = indexes->getSize().y;
        setSize(sf::Vector2i(tileX * width, tileY * height));
        setOrigin(0, 0);

        //Set up buffer texture
        buffer = new sf::RenderTexture();
        if(!buffer->create(tileX * width, tileY * height))
            throw std::logic_error("Error creating TileMap buffer");

        // resize the vertex array to fit the level size
        vertices.setPrimitiveType(sf::Quads);
        vertices.resize(width * height * 4);

        //Load textures
        reload(offset);
    }

    ~TileMap() {
    }

    int countTextures() {
        return (tileset->getSize().x / tileX) * (tileset->getSize().y / tileY);
    }

    void reload(int offset = 0) {
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