# Events

All inputs pass through the event system, as well as most of networking. Custom events are also easy to use. While some fields can go unused, every event uses the exact same struct.

Events are added to a global queue in UpdateList, duplicate events are filtered out there, (Some events may store a counter to get past that). Nodes (or UNode) must register with `UpdateList::addListener()` to receive events of a specific type. Before each update, the queue is emptied calling `recieveEvent()` on all registered listeners. Custom events can be added to the queue with `UpdateList::queueEvent()`

### Types

EVENT_ Type 			| Code 					| Down 			| x,y 			
------------------------|-----------------------|---------------|---------------
KEYPRESS				| Keycode (Keymap.cpp)	| Key pressed	| 
MOUSE					| 0=L, 1=R, 2=M, etc.	| Button pressed| Mouse position
SCROLL					| 						| Downwards		| Wheel movement
TOUCH					| 0 to number of touches| Touched		| Touch position
JOYSTICK				| Stick 1-4 + controller| Non-zero		| Stick position
JOYSTICK_SIM			| Same as Joystick 		| In use		| Touchscreen joystick position
RESIZE					| RenderW / ScreenW 	| First frame	| Screen size
FOCUS					| 		 				| Unfocused		|
SUSPEND					| 		 				| Window Hidden	|
SETTINGS				| Changes counter 		| Saved to file |
BUFFER					| Buffer resource ID	| Draw finished	|
IMGUI					| 						| Menu Bar		|
NETWORK_CONNECT_SERVER	| Your new Client ID	| Disconnect	|
NETWORK_CONNECT_CLIENT	| Client ID				| Disconnect	|


The remaining event types are provided for convenience. `EVENT_NETWORK_CONNECT_CLIENT` is sent automatically by the default server on disconnect, but it is merely a recomendation to send it with custom data on connecting. Any event type greater than `EVENT_CUSTOM` is passed to `EVENT_CUSTOM` listeners.

```
EVENT_NETWORK_CONNECT_CLIENT
EVENT_NETWORK_POSITION1-4
EVENT_NETWORK_CUSTOM1-4
EVENT_CUSTOM1-4
EVENT_CUSTOM
```

### Networking
The default server provides basic broadcast passing of events and strings between 1-4 clients.

1. Call `UpdateList::connectServer(ip, port)` to attempt a connection.
2. If successful, it will reply with a `EVENT_NETWORK_CONNECT_SERVER : false` event.
3. The client ID is automatically saved to `UpdateList::getNetworkId()`.
4. It is recomended to send a `EVENT_NETWORK_CONNECT_CLIENT : false` message with `UpdateList::sendNetworkEvent()`, x and y can be anything.
5. Other clients may send their own `EVENT_NETWORK_CONNECT_CLIENT` messages.
6. For simplicity/completeness, resend your `EVENT_NETWORK_CONNECT_CLIENT : false` message whenever an uknown `EVENT_NETWORK_CONNECT_CLIENT` message is received.
7. More events can be sent with `UpdateList::sendNetworkEvent()`, though any type less than `EVENT_NETWORK_CONNECT_CLIENT` will be rejected.
8. Events will show up in the normal event queue and must be listened for.
9. Strings can be sent with `UpdateList::sendNetworkString`, along with a numerical code.
10. Strings will be recieved by the global undefined function `recieveNetworkString`
11. Disconnecting can be forced with `UpdateList::disconnectServer()`.
12. Disconnecting from the server will create a `EVENT_NETWORK_CONNECT_SERVER : true` event locally, and all other clients will recieve a `EVENT_NETWORK_CONNECT_CLIENT : true` message from the server.

### Signals
Signals are a similar but separate and more direct system of passing data between nodes. They are never used by the Engine. Call `UpdateList::sendSignal()` to send a number and node pointer, either to all nodes in a specific layer, or to all nodes at once. It will call `recieveSignal()` on all of them, potentially including itself.

### Sources
- [Event.h](https://github.com/stuin/Skyrmion/blob/main/core/Event.h)
- [Keylist.cpp](https://github.com/stuin/Skyrmion/blob/main/input/Keylist.cpp)
- [UpdateList.h](https://github.com/stuin/Skyrmion/blob/main/core/UpdateList.h)
- [nbnetServer.cpp](https://github.com/stuin/Skyrmion/blob/main/core/backend/nbnetServer.cpp)