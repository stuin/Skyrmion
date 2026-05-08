# Skyrmion
A 2D Game Engine built in C++, very focused on tilemaps.
Currently built on top of [Raylib](https://github.com/raysan5/raylib), with support for PC/Web/Android. (Can also be compiled with [Sokol](https://github.com/floooh/sokol)).

Many dependencies included as submodules, including: [Dear ImGui](https://github.com/ocornut/imgui), [nlohmann json](https://json.nlohmann.me/) and [libnoise](https://libnoise.sourceforge.net/).

## Tilemaps:
Most of the focus of the engine is on Grids which can be used for collision, lighting, and placing other objects. These can be rendered by Tilemaps, which can be animated, offset and layered in different ways to provide many visual effects from one grid. Full information on those [here](https://github.com/stuin/Skyrmion/blob/main/docs/Tiles.md).

## Nodes:
Everything visible in the game is a Node.
Each node is attached to a specific layer in UpdateList, usually ordered and named by an enum, which decides render, collision, and update order. Full information on those [here](https://github.com/stuin/Skyrmion/blob/main/docs/Nodes.md).

## Backends:
- Include `core/backend/RaylibUpdateList.cpp` to compile for Raylib
- Include `core/backend/SokolUpdateList.cpp` and `SokolAudio.cpp` to compile for Sokol
- Most functionality should be identical between them
- Originally built using SFML

## DearImGui debug windows:
- FPS counters for draw thread and update thread, with both real times and theoretical unlimited times
- List of layers with names and flags
- Debug information and collision box for individual nodes
- Stream of latest events and inputs
- List of textures and other resources
- Settings and controls
- Modifyable perlin noise generator

## Other tools:
- Json settings file with static/global reading and writing  
- Json customizable controls supporting keyboard/mouse/gamepad buttons, joystick movement, and up to 3 binds for every action
- N dimensional directed edge-vertex graph
- Basic networking with a server to pass events between clients

## Example games:
- The engine was originally built with a group as the core systems of [Temple-of-Pele](https://github.com/skyrmiongames/Temple-of-Pele), and later extracted into it's own project. As more game jam type projects were created with it I added more features
- Lighting was added with [The Path Below](https://stuin.itch.io/the-path-below) : [Source](https://github.com/stuin/ThePathBelow)
- For GMTK 2022, Roll of the Dice, I created [Rolling Labyrinth](https://stuin.itch.io/rolling-labyrinth) : [Source](https://github.com/stuin/RollingLabyrinth)
- For GMTK 2024 Built to Scale, I designed a side-scrolling physics engine (Soon to be moved into the engine proper) [Climbing Blocks](https://stuin.itch.io/climbing-blocks) : [Source](https://github.com/stuin/ClimbingBlocks)