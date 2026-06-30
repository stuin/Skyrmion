#pragma once

#include <string>
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
	int type = SK_INVALID;

	ResourceData(std::string _filename, int _type) {
		filename = _filename;
		type = _type;
	}

	ResourceData(std::string _filename, int _type, Vector2i _size, sint _index=0) {
		filename = _filename;
		type = _type;
		size = _size;
		index = _index;
	}

	bool isTexture() {
		return type == SK_TEXTURE || type == SK_BUFFER;
	}
};

//Buffer draw data
struct BufferData {
	sint texture;
	sint shader = 0;
	Vector2i size;
	std::bitset<MAXLAYER> layers;
	Node *source = NULL;
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

	BufferData(sint _texture, Vector2i _size, int _layer, skColor _color = COLOR_WHITE) {
		texture = _texture;
		size = _size;
		if(_layer >= 0)
			layers[_layer] = true;
		color = _color;
	}

	BufferData(sint _texture, Node *_node, skColor _color = COLOR_WHITE) {
		texture = _texture;
		size = _node->getSize();
		source = _node;
		color = _color;
	}
};

struct ShaderUniform {
	sint texture = 0;
	sint shader;
	std::string name;
	std::vector<float> fValues;
	std::vector<int> iValues;
	int type;
	int location = -1;
	bool update = true;

	ShaderUniform(sint _shader, std::string _name, std::vector<float> _values, int _type) {
		shader = _shader;
		name = _name;
		fValues = _values;
		type = _type;
	}
	ShaderUniform(sint _shader, std::string _name, std::vector<int> _values, int _type) {
		shader = _shader;
		name = _name;
		iValues = _values;
		type = _type;
	}

	bool isFloat() {
		return type == SKU_FLOAT || type == SKU_FLOAT3_VECTOR || type == SKU_FLOAT2;
	}
};

class UpdateList {
private:
	//Node management
	static LayerData layers[MAXLAYER];
	static UNode* uLayers[MAXLAYER];
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
	static bool remapKeycode;

	//Viewport variables
	static Node *camera;
	static FloatRect cameraRect;
	static FloatRect screenRect;
	static skColor backgroundColor;

	//Skyrmion Resource Data
	static std::vector<ResourceData> resourceData;
	static std::vector<BufferData> bufferData;
	static std::vector<ShaderUniform> shaderUniforms;

	//Networking
	static bool networkInitialized;
	static bool networkClient;
	static bool networkConnected;
	static int networkId;
	static unsigned int networkTimer;

	//Private internal functions
	static int loadResource(std::string filename);
	static void queueEvents();
	static void processAudio();
	static void drawNode(Node *source, sint passthrough=0);
	static void draw(FloatRect cameraRect);
	static void drawBuffer(sint buffer);
	static void sendUniformValues(sint uniform);
	static void update(double time);
	static void frame(void);
	static void cleanup(void);
	static void processNetworking();
	static void processNetworkMessage();

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
	static void startRemap();
	static bool checkKeycode(int keycode, bool down);
	static void queueEvent(Event event);
	static void sendSignal(int layer, int id, Node *sender);
	static void sendSignal(int id, Node *sender);

	//Screen view
	static Node *setCamera(Node *follow, Vector2f size, Vector2f position=Vector2f(0,0));
	static FloatRect getCameraRect();
	static FloatRect getScreenRect();
	static Vector2i getScreenSize();
	static Vector2f getScaleFactor();

	//Layer control features
	static void pauseLayer(int layer, bool pause=true);
	static void hideLayer(int layer, bool hidden=true);
	static void globalLayer(int layer, bool global=true);

	//Layer read features
	static bool isLayerPaused(int layer);
	static bool isLayerHidden(int layer);
	static LayerData &getLayerData(int layer);
	static sint getLayerCount();

	//Resource handling
	static sint createResource(sint texture, Vector2i size, sint index, int type);
	static ResourceData &getResourceData(sint index);
	static sint getResourceCount();
	static Vector2i getTextureSize(sint index);
	static void drawImGuiTexture(sint texture, Vector2i size);
	static skColor pickColor(sint texture, Vector2i position);
	static sint createBuffer(BufferData data);
	static void scheduleBufferRefresh(sint buffer);
	static BufferData &getBufferData(sint buffer);

	//Shader Uniforms
	static sint createUniform(sint rIndex, sint shader, std::string name, std::vector<float> values);
	static sint createUniform(sint rIndex, sint shader, std::string name, std::vector<int> values, int type=SKU_INT3_VECTOR);
	static sint createUniform(sint rIndex, sint shader, std::string name, float value);
	static sint createUniform(sint rIndex, sint shader, std::string name, int value);
	static sint createUniform(sint rIndex, sint shader, std::string name, Vector2f value);
	static void updateUniform(sint uniform, std::vector<float> values);
	static void updateUniform(sint uniform, std::vector<int> values);
	static void updateUniform(sint uniform, float value);
	static void updateUniform(sint uniform, int value);
	static void updateUniform(sint uniform, Vector2f value);
	static ShaderUniform &getUniform(sint uniform);

	//Start engine
	static void startEngine();
	static void stopEngine();
	static bool isRunning();

	//Semi private internal functions
	static void processEvents();
	static void init(void);

	//Audio controls
	static void setVolume(int volume);
	static void musicStream(std::string filename, int volume=100);

	//Networking client functions
	static void connectServer(std::string ip, int port);
	static void disconnectServer();
	static bool isConnected();
	static int getNetworkId();
	static bool isNetworkTick();
	static void sendNetworkEvent(Event event, bool reliable=false);
	static void sendNetworkString(std::string data, int code=0, bool reliable=true);
};

//Config options for pre-init setup
struct WindowConfig {
	std::string windowTitle;
	Vector2i windowSize;
	skColor backgroundColor;
	std::vector<std::string> &textureFiles;
	std::vector<std::string> &layerNames;
};

//System functions to be implemented by the game
WindowConfig windowConfig();
void initialize();

//Networking functions to be implemented by the game
void recieveNetworkString(std::string data, int code);

//Debug tool insertions
void setupDebugTools();

void stream_cb(float* buffer, int num_frames, int num_channels);