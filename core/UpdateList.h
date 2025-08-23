#pragma once

#include <string>
#include <vector>
#include <deque>

#include "Node.h"

/*
 * Manages list of nodes through update cycle
 */

//Data for each layer
struct LayerData {
	std::string name = "";
	bool paused = false;
	bool hidden = false;
	bool global = false;
	Node *root = NULL;
	int count = 0;
};

//Buffer draw data
struct BufferData {
	sint texture;
	Vector2i size;
	std::bitset<MAXLAYER> layers;
	skColor color;
	bool redraw = true;

	BufferData() {
		texture = 0;
		redraw = false;
	}

	BufferData(sint _texture, Vector2i _size, std::bitset<MAXLAYER> _layers, skColor _color = COLOR_WHITE) {
		texture = _texture;
		size = _size;
		layers = _layers;
		color = _color;
	}

	BufferData(sint _texture, Vector2i _size, Layer _layer, skColor _color = COLOR_WHITE) {
		texture = _texture;
		size = _size;
		layers[_layer] = true;
		color = _color;
	}
};

//Metadata surrounding each texture
struct TextureData {
	std::string filename = "";
	Vector2i size;
	bool valid = false;
	sint buffer = 0;

	TextureData(std::string _filename) {
		filename = _filename;
	}

	TextureData(std::string _filename, Vector2i _size) {
		filename = _filename;
		size = _size;
		buffer = 0;
		valid = true;
	}
};

class UpdateList {
private:
	//Node management
	static LayerData layers[MAXLAYER];
	static Layer maxLayer;
	static bool running;
	static std::vector<Node *> deleted1;
	static std::vector<Node *> deleted2;

	//Viewport variables
	static Node *camera;
	static FloatRect cameraRect;
	static FloatRect screenRect;

	//Networking
	static bool networkInitialized;
	static bool networkClient;
	static bool networkConnected;
	static int networkId;
	static unsigned int networkTimer;

public:

	//Manage node lists
	static void addNode(Node *next);
	static void addNodes(std::vector<Node *> nodes);
	static Node *getNode(Layer layer);
	static void clearLayer(Layer layer);

	//Events and signals
	static void addListener(Node *item, int type);
	static void watchKeycode(int keycode);
	static void queueEvent(Event event);
	static void sendSignal(Layer layer, int id, Node *sender);
	static void sendSignal(int id, Node *sender);

	//Screen view
	static Node *setCamera(Node *follow, Vector2f size, Vector2f position=Vector2f(0,0));
	static FloatRect getCameraRect();
	static FloatRect getScreenRect();
	static Vector2f getScaleFactor();

	//Layer control features
	static void pauseLayer(Layer layer, bool pause=true);
	static void hideLayer(Layer layer, bool hidden=true);
	static void globalLayer(Layer layer, bool global=true);

	//Layer read features
	static bool isLayerPaused(Layer layer);
	static bool isLayerHidden(Layer layer);
	static LayerData &getLayerData(Layer layer);
	static int getLayerCount();

	//Texture Handling
	static int loadTexture(std::string filename);
	static int createBuffer(BufferData data);
	static int createBuffer(sint _texture, Vector2i _size, Layer _layer, skColor _color = COLOR_WHITE);
	static void scheduleReload(sint buffer);
	static Vector2i getTextureSize(sint index);
	static TextureData &getTextureData(sint index);
	static void drawImGuiTexture(sint texture, Vector2i size);
	static skColor pickColor(sint texture, Vector2i position);

	//Start engine
	static void startEngine();
	static void stopEngine();
	static bool isRunning();

	//Main loop functions
	static void processEvents();
	static void queueEvents();
	static void processAudio();
	static void drawNode(Node *source);
	static void draw(FloatRect cameraRect);
	static void drawBuffer(BufferData data);
	static void update(double time);
	static void frame(void);
	static void init(void);
	static void cleanup(void);

	//Audio controls
	static void setVolume(int volume);
	static void musicStream(std::string filename, int volume=100);

	//Networking client functions
	static void connectServer(std::string ip, int port);
	static void disconnectServer();
	static bool isConnected();
	static int getNetworkId();
	static bool isNetworkTick();
	static void processNetworking();
	static void processNetworkMessage();
	static void sendNetworkEvent(Event event, bool reliable=false);
	static void sendNetworkString(std::string data, int code=0, bool reliable=true);
};

//System functions to be implemented by the game
void initialize();
skColor backgroundColor();
std::string *windowTitle();
std::vector<std::string> &textureFiles();
std::vector<std::string> &layerNames();

//Networking functions to be implemented by the game
void recieveNetworkString(std::string data, int code);

//Debug tool insertions
void setupDebugTools();
void addDebugTextures();

void stream_cb(float* buffer, int num_frames, int num_channels);