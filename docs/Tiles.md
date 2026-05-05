# Tiles
There are two parts to the tile system.

### GridMaker
A GridMaker stores a grid of integer tiles, often loaded from a txt file or modified by code. A grid can then be passed through various Indexers, which can read the tiles in different ways to define specific properties. (Ex. `T` can map to 1 for rendering, 0 for collision, and 100 for lighting purposes). Indexers can also be set for Perlin noise or other modifiers.

### TileMap
A TileMap is the standard node used to render an Indexer from a Grid, allowing for:

- Transparency
- Animations
- Rotated and flipped textures
- Tiles with different properties sharing the same texture
- Invisible tiles
- Changing tiles during runtime
- Randomly rotating or choosing between multiple textures
- Choosing textures based on intersections between multiple tiles (Autotiling)
- Multiple layers rendering below and on top of other objects
- Using multiple layers to visually extend into other tiles
- Buffering to a render texture
- Splitting up a large tilemap into smaller sections

### Sources
- [GridMaker.h](https://github.com/stuin/Skyrmion/blob/main/tiling/GridMaker.h)
- [TileMap.hpp](https://github.com/stuin/Skyrmion/blob/main/tiling/TileMap.hpp)
- [LightMap.h](https://github.com/stuin/Skyrmion/blob/main/tiling/LightMap.h)