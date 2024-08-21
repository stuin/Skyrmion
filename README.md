# Skyrmion
A 2D Game Engine built in C++, very focused on tilemaps.
Requires [SFML](https://www.sfml-dev.org/) and [nlohmann json](https://json.nlohmann.me/).

## Tilemaps

Load from txt files as ASCII art, each char representing a tile and it's properties, or randomly generate tiles and modify them during the game. Or both. It is predrawn to one or more render textures based on size, but can be redrawn later.

- Collision
- Transparency
- Animations
- Dynamic Lighting (emmision, shadow casting, and transparent shadow casting)
- Rotated and flipped textures
- Tiles with different properties sharing the same texture
- Invisible tiles
- Changing tiles during runtime
- Randomly choosing between mutiple textures
- Multiple layers rendering below and on top of other objects
- Using multiple layers to visually extend into other tiles
- Spawning objects at specific locations on startup

#### Sources
- [GridMaker.h](https://github.com/stuin/Skyrmion/blob/main/tiling/GridMaker.h)
- [TileMap.hpp](https://github.com/stuin/Skyrmion/blob/main/tiling/TileMap.hpp)
- [LightMap.h](https://github.com/stuin/Skyrmion/blob/main/tiling/LightMap.h)

## Nodes:
Everything visible in the game is a Node or rendered by a Node. These are SFML sprites with extra features.
Each node is attached to a specific layer in UpdateList, usually ordered and named by an enum, which decides render, collision, and update order.

- Can display textures, simple spritesheet animations, buffered render textures, or builtin SFML objects natively
- Can also just be given a completely custom draw function
- Can be hidden while still updating
- Only rendered when on screen
- Specific layers can keep updating while off screen
- Layers can be paused while still being visible, or hidden without being paused
- Nodes can be deleted on mass by layer
- Camera can be static or attached to any node
- Parent a node to any other node in a tree
- Uses centered position by default
- Set and get position relative to parent or globally
- Limited light emmision
- Collision with tiles
- Collision with other nodes by layer
- Send signals to any nodes by layer
- Subscribe to window events by type (resizing, mouse, keyboard, etc)
- Thread safe deletion and render texture drawing

Updates are run as often as possible, with a time delta variable provided for consistency. Draw calls are done in a separate  read-only thread at maximum ~60 fps.

#### Sources
- [Node.h](https://github.com/stuin/Skyrmion/blob/main/Node.h)
- [UpdateList.h](https://github.com/stuin/Skyrmion/blob/main/UpdateList.h)

## Other tools:

- Automatic window/render resizing
- Static/global quit game function
- Json settings file static/global reading and writing  
- Json customizable controls supporting keyboard/mouse/gamepad buttons, joystick movement, and up to 3 binds for every action
- Extra vector functions for finding+setting length and simpler multiplication
- N dimensional directed edge-vertex graph
- Canvas to draw circles and text at any location

#### Sources
- [Settings.h](https://github.com/stuin/Skyrmion/blob/main/input/Settings.h)
- [Keylist.cpp](https://github.com/stuin/Skyrmion/blob/main/input/Keylist.cpp)
- [VertexGraph.hpp](https://github.com/stuin/Skyrmion/blob/main/util/VertexGraph.hpp)
- [Canvas.hpp](https://github.com/stuin/Skyrmion/blob/main/util/Canvas.hpp)

## Example games:
- The engine was originally built with a group as the core systems of [Temple-of-Pele](https://github.com/skyrmiongames/Temple-of-Pele), and later extracted into it's own project. As more game jam type projects were created with it I added more features
- Lighting was added with [The Path Below](https://stuin.itch.io/the-path-below) : [Source](https://github.com/stuin/ThePathBelow)
- For GMTK 2022, Roll of the Dice, I created [Rolling Labyrinth](https://stuin.itch.io/rolling-labyrinth) : [Source](https://github.com/stuin/RollingLabyrinth)
- For GMTK 2024 Built to Scale, I designed a side-scrolling physics engine (Soon to be moved into the engine proper) [Climbing Blocks](https://stuin.itch.io/climbing-blocks) : [Source](https://github.com/stuin/ClimbingBlocks)