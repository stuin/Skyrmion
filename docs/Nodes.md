# Node

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

### RenderComponent
Some texture data is stored separatly from the Node in a RenderComponent. This is generally set by the constructor but can be replaced later.

RENDER_ type		| Description
--------------------|-------------
NONE 				| Nothing
TEXTURE_SINGLE		| Entire texture
TEXTURE_RECT 		| Specific rectangle from texture
TEXTURE_ARRAY		| Multiple rectangles from texture
TEXTURE_MAP			| Indexes for Tilemap
COLOR_SINGLE		| Single solid color
COLOR_RECT			| Rectangle with optional border
COLOR_MAP			| Array of colors
PASSTHROUGH_BUFFER	| Stores a second RenderComponent to render onto a buffer
STRING				| Text

### UNode
A UNode is a simplified Node with no rendering or position, just updates and events. They are stored in separate layers, either with a negative number to update before the normal Nodes, or a positive layer to update after.

Updates are run at ~100 per second, with a time delta variable provided for consistency. Draw calls are done in a separate read-only thread.

### Other Docs
- [Events System](https://github.com/stuin/Skyrmion/blob/main/docs/Events.md)
- [TileMaps](https://github.com/stuin/Skyrmion/blob/main/docs/Tiles.md)

### Sources
- [Node.h](https://github.com/stuin/Skyrmion/blob/main/core/Node.h)
- [UpdateList.h](https://github.com/stuin/Skyrmion/blob/main/core/UpdateList.h)
- [Vector.h](https://github.com/stuin/Skyrmion/blob/main/core/Vector.h)

