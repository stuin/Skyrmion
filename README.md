# Skyrmion
A 2D Game Engine built in C++, very focused on tilemaps.
Previously designed for SFML, now uses Sokol (A branch also exists using Raylib).
Requires installing [nlohmann json](https://json.nlohmann.me/) and [libnoise](https://libnoise.sourceforge.net/).
Also includes as submodules [Sokol](https://github.com/floooh/sokol), [Sokol GP](https://github.com/edubart/sokol_gp), and [Dear ImGui](https://github.com/ocornut/imgui).

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
- Randomly rotating or choosing between multiple textures
- Perlin noise and related options provided by libnoise
- Multiple layers rendering below and on top of other objects
- Using multiple layers to visually extend into other tiles
- Spawning objects at specific locations on startup

#### Sources
- [GridMaker.h](https://github.com/stuin/Skyrmion/blob/main/tiling/GridMaker.h)
- [TileMap.hpp](https://github.com/stuin/Skyrmion/blob/main/tiling/TileMap.hpp)
- [LightMap.h](https://github.com/stuin/Skyrmion/blob/main/tiling/LightMap.h)

## Nodes:
Everything visible in the game is a Node or rendered by a Node.
Each node is attached to a specific layer in UpdateList, usually ordered and named by an enum, which decides render, collision, and update order.

- float Position, Size, and Scaling
- Has Texture, Origin, BlendMode, and Texture Rectangles
- Can display textures, simple spritesheet animations, buffered render textures
- Can be hidden while still updating
- By default nodes are only rendered when on screen
- Layers can be paused while still being visible, or hidden without being paused
- Layers can use global coordinates or can be placed relative to screen
- Thread safe node deletion
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

Updates are run at ~100 per second, with a time delta variable provided for consistency. Draw calls are done in a separate read-only thread.

#### Sources
- [Node.h](https://github.com/stuin/Skyrmion/blob/main/core/Node.h)
- [UpdateList.h](https://github.com/stuin/Skyrmion/blob/main/core/UpdateList.h)

## Dear ImGui Debug Tools:
- List of layers with names and flags
- Debug information for each node
- Live updating perlin noise generator
- Color picker including from loaded textures

## Other tools:

- Automatic window/render resizing
- Static/global quit game function
- Json settings file static/global reading and writing  
- Json customizable controls supporting keyboard/mouse/gamepad buttons, joystick movement, and up to 3 binds for every action
- Extra vector functions for finding+setting length and simpler multiplication
- N dimensional directed edge-vertex graph

#### Sources
- [Settings.h](https://github.com/stuin/Skyrmion/blob/main/input/Settings.h)
- [Keylist.cpp](https://github.com/stuin/Skyrmion/blob/main/input/Keylist.cpp)
- [VertexGraph.hpp](https://github.com/stuin/Skyrmion/blob/main/util/VertexGraph.hpp)

## Example games:
- The engine was originally built with a group as the core systems of [Temple-of-Pele](https://github.com/skyrmiongames/Temple-of-Pele), and later extracted into it's own project. As more game jam type projects were created with it I added more features
- Lighting was added with [The Path Below](https://stuin.itch.io/the-path-below) : [Source](https://github.com/stuin/ThePathBelow)
- For GMTK 2022, Roll of the Dice, I created [Rolling Labyrinth](https://stuin.itch.io/rolling-labyrinth) : [Source](https://github.com/stuin/RollingLabyrinth)
- For GMTK 2024 Built to Scale, I designed a side-scrolling physics engine (Soon to be moved into the engine proper) [Climbing Blocks](https://stuin.itch.io/climbing-blocks) : [Source](https://github.com/stuin/ClimbingBlocks)