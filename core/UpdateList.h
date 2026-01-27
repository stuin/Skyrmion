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
	sint shader = 0;
};

//Metadata surrounding each resource
struct ResourceData {
	std::string filename = "";
	Vector2i size;
	sint index = 0;
	SK_RESOURCE_TYPE type = SK_INVALID;

	ResourceData(std::string _filename, SK_RESOURCE_TYPE _type) {
		filename = _filename;
		type = _type;
	}

	ResourceData(std::string _filename, SK_RESOURCE_TYPE _type, Vector2i _size, sint _index=0) {
		filename = _filename;
		type = _type;
		size = _size;
		index = _index;
	}
};

class UpdateList {
private:
	//Node management
	static LayerData layers[MAXLAYER];
	static UNode* uLayers[MAXLAYER*2];
	static int maxLayer;
	static int maxULayer;
	static bool running;
	static std::vector<UNode *> deleted1;
	static std::vector<UNode *> deleted2;

	//Event handling
	static std::array<std::vector<UNode *>, EVENT_MAX> listeners;
	static std::deque<Event> event_queue;
	static std::array<Event, EVENT_MAX> event_previous;
	static std::vector<int> watchedKeycodes;
	static std::vector<bool> watchedKeycodesPrevious;

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
	static Node *getNode(int layer);
	static void clearLayer(int layer);

	static void addUNode(UNode *next);
	static UNode *getUNode(int layer);

	//Events and signals
	static void addListener(UNode *item, int type);
	static void watchKeycode(int keycode);
	static void queueEvent(Event event);
	static void sendSignal(int layer, int id, Node *sender);
	static void sendSignal(int id, Node *sender);

	//Screen view
	static Node *setCamera(Node *follow, Vector2f size, Vector2f position=Vector2f(0,0));
	static FloatRect getCameraRect();
	static FloatRect getScreenRect();
	static Vector2f getScaleFactor();

	//Layer control features
	static void pauseLayer(int layer, bool pause=true);
	static void hideLayer(int layer, bool hidden=true);
	static void globalLayer(int layer, bool global=true);

	//Layer read features
	static bool isLayerPaused(int layer);
	static bool isLayerHidden(int layer);
	static LayerData &getLayerData(int layer);
	static int getLayerCount();

	//Texture Handling
	static int loadResource(std::string filename);
	static int createBuffer(sint _texture, Vector2i _size, std::bitset<MAXLAYER> _layers, Node *source=NULL, sint shader=0, skColor _color = COLOR_WHITE);
	static int createBuffer(sint _texture, Vector2i _size, int _layer, skColor _color = COLOR_WHITE);
	static int createBuffer(sint _texture, Node *_node, skColor _color = COLOR_WHITE);
	static void scheduleBufferRefresh(sint buffer);
	static Vector2i getTextureSize(sint index);
	static ResourceData &getResourceData(sint index);
	static sint getResourceCount();
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
	static void drawNode(Node *source, sint passthrough=0);
	static void draw(FloatRect cameraRect);
	static void drawBuffer(sint buffer);
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