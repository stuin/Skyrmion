#pragma once

#include "../core/Node.h"
#include "../tiling/GridMaker.h"

#include <vector>
#include <stdexcept>

/*
 * Render raw colors as tiles
 */

class ColorMap : public Node {
private:
    Indexer *indexes;
    uint gridUpdates = 0;

    std::function<skColor(int)> func;

    uint fullWidth = 0;
    uint fullHeight = 0;
    uint width = 0;
    uint height = 0;
    uint startX = 0;
    uint startY = 0;

public:
    ColorMap(Indexer *_indexes, std::function<skColor(int)> _func, int layer=0, Rect<uint> border=Rect<uint>())
     : Node(layer, RENDER_COLOR_ARRAY), indexes(_indexes), func(_func) {

        //Set sizing
        fullWidth = width = indexes->getSize().x * indexes->getScale().x;
        fullHeight = height = indexes->getSize().y * indexes->getScale().x;
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

        setSize(Vector2i(width, height));
        setOrigin(0, 0);
        setPosition(startX, startY);

        //setTexture(_tileset);
        setupBuffer(0, COLOR_EMPTY);

        //std::cout << " " << startX << "," << startY << ", " << width << "," << height << "\n";
        //std::cout << toString(getGPosition()) << ":" << toString(getGScale()) <<  "\n";

        getRenderComponent()->getColors()->reserve(width * height);
        getRenderComponent()->setSize(width);

        //Load textures
        reload();
    }

    ~ColorMap() {
    }

    void reload() {
        int usedRects = 0;
        bool hasBuffer = true;

        // populate the vertex array, with one quad per tile
        for(unsigned int j = 0; j < height; ++j) {
            for(unsigned int i = 0; i < width; ++i) {
                // get the current tile number
                int tileValue = indexes->getTile(Vector2f(i + startX, hasBuffer ? fullHeight - (j + startY + 1) : j + startY));
                skColor tileColor = func(tileValue);
                (*getRenderComponent()->getColors())[usedRects++] = tileColor;
                //std::cout << tileValue << tileColor << ' ';
            }
            //std::cout << "\n";
        }
        gridUpdates = indexes->getUpdateCount();
        getRenderComponent()->getColors()->resize(usedRects);
        //std::cout << getRenderComponent()->getColors()->size();

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

std::function<skColor(int)> percentColorFunc(int max=100) {
    return [max](int v) {
        float c = v*1.0f/max;
        return skColor(c,c,c);
    };
}