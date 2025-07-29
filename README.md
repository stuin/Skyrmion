# Skyrmion
A 2D Game Engine built in C++, very focused on tilemaps.
Currently built on top of [Raylib](https://github.com/raysan5/raylib), with support for PC/Web/Android. (Can also be compiled with [Sokol](https://github.com/floooh/sokol)).

Many dependencies included as submodules, including: [Dear ImGui](https://github.com/ocornut/imgui), [nlohmann json](https://json.nlohmann.me/) and [libnoise](https://libnoise.sourceforge.net/).

## Tilemaps
Load from txt files as ASCII art, each char representing a tile and it's properties, or randomly generate tiles and modify them during the game.

- Collision
- Transparency
- Animations
- Rotated and flipped textures
- Tiles with different properties sharing the same texture
- Invisible tiles
- Changing tiles during runtime
- Dynamic Lighting (emmision and shadow casting)
- Randomly rotating or choosing between multiple textures
- Perlin noise and related options provided by libnoise
- Choosing textures based on intersections between multiple tiles (Autotiling)
- Multiple layers rendering below and on top of other objects
- Using multiple layers to visually extend into other tiles
- Spawning objects at specific locations on startup
- Buffering to a render texture

#### Sources
- [GridMaker.h](https://github.com/stuin/Skyrmion/blob/main/tiling/GridMaker.h)
- [TileMap.hpp](https://github.com/stuin/Skyrmion/blob/main/tiling/TileMap.hpp)
- [LightMap.h](https://github.com/stuin/Skyrmion/blob/main/tiling/LightMap.h)

## Nodes:
Everything visible in the game is a Node.
Each node is attached to a specific layer in UpdateList, usually ordered and named by an enum, which decides render, collision, and update order.

- Vectors for Position, Origin, Size, and Scaling
- Textures are stored separatly and referenced with a global id
- Texture Rectangles allow for rendering sections of textures with transformations
- Support for animations, tilemaps, rotations
- Can replace texture with text rendering
- Can be hidden while still updating
- By default nodes are only rendered when on screen
- Layers can be paused while still being visible, or hidden without being paused
- Thread safe node deletion
- Nodes can be deleted on mass by layer
- Camera can be static or attached to any node
- Parent a node to any other node in a tree
- Uses centered position by default
- Set and get position relative to parent or globally
- Set position by screen coordinates
- Collision with tiles
- Collision with other nodes by layer
- Send signals to any nodes by layer
- Subscribe to input/window events by type (resizing, mouse, keyboard, etc)
- Thread safe deletion and render texture drawing

Updates are run at ~100 per second, with a time delta variable provided for consistency. Draw calls are done in a separate read-only thread.

#### Sources
- [Node.h](https://github.com/stuin/Skyrmion/blob/main/core/Node.h)
- [UpdateList.h](https://github.com/stuin/Skyrmion/blob/main/core/UpdateList.h)

## Backends
- Include `core/backend/RaylibUpdateList.cpp` to compile for Raylib
- Include `core/backend/SokolUpdateList.cpp` and `SokolAudio.cpp` to compile for Sokol
- Most functionality should be identical between them
- Originally built using SFML

## DearImGui debug windows:
- FPS counters for draw thread and update thread, with both real times and theoretical unlimited times
- List of layers with names and flags
- Debug information and collision box for individual nodes
- Stream of latest events and inputs
- Modifyable perlin noise generator
- Color picker with list of loaded textures

## Other tools:
- Json settings file with static/global reading and writing  
- Json customizable controls supporting keyboard/mouse/gamepad buttons, joystick movement, and up to 3 binds for every action
- N dimensional directed edge-vertex graph
- Basic networking with a server to pass events between clients

#### Sources
- [Settings.h](https://github.com/stuin/Skyrmion/blob/main/input/Settings.h)
- [Keylist.cpp](https://github.com/stuin/Skyrmion/blob/main/input/Keylist.cpp)
- [VertexGraph.hpp](https://github.com/stuin/Skyrmion/blob/main/util/VertexGraph.hpp)
- [nbnetServer.cpp](https://github.com/stuin/Skyrmion/blob/main/core/backend/nbnetServer.cpp)

## Example games:
- The engine was originally built with a group as the core systems of [Temple-of-Pele](https://github.com/skyrmiongames/Temple-of-Pele), and later extracted into it's own project. As more game jam type projects were created with it I added more features
- Lighting was added with [The Path Below](https://stuin.itch.io/the-path-below) : [Source](https://github.com/stuin/ThePathBelow)
- For GMTK 2022, Roll of the Dice, I created [Rolling Labyrinth](https://stuin.itch.io/rolling-labyrinth) : [Source](https://github.com/stuin/RollingLabyrinth)
- For GMTK 2024 Built to Scale, I designed a side-scrolling physics engine (Soon to be moved into the engine proper) [Climbing Blocks](https://stuin.itch.io/climbing-blocks) : [Source](https://github.com/stuin/ClimbingBlocks)