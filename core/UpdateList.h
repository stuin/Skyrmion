#pragma once

#include <atomic>
#include <bitset>
#include <chrono>
#include <deque>
#include <exception>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Node.h"
#include "Color.h"

/*
 * Manages list of nodes through update cycle
 */

struct LayerData {
	std::string name = "";
	bool paused = false;
	bool hidden = false;
	bool global = false;
	bool screenSpace = false;
	Node *root = NULL;
	int count = 0;
};

struct TextureData {
	std::string filename = "";
	Vector2i size;
	bool buffer = false;
	bool valid = false;

	TextureData(std::string _filename) {
		filename = _filename;
	}

	TextureData(std::string _filename, Vector2i _size, bool _buffer=false) {
		filename = _filename;
		size = _size;
		buffer = _buffer;
		valid = true;
	}
};

class UpdateList {
private:
	//Node management
	static LayerData layers[MAXLAYER];
	static Layer maxLayer;
	static bool running;
	static std::vector<Node *> deleted;

	//Viewport variables
	static Node *camera;
	static FloatRect viewport;
	static std::vector<Node *> reloadBuffer;

	//Window event system
	//static std::atomic_int event_count;
	//static std::deque<Event> event_queue;
	//static std::array<std::vector<Node *>, EVENT_MAX> listeners;

public:

	//Manage node lists
	static void addNode(Node *next);
	static void addNodes(std::vector<Node *> nodes);
	static Node *getNode(Layer layer);
	static void clearLayer(Layer layer);
	static void addListener(Node *item, int type);

	//Special node features
	static Node *setCamera(Node *follow, Vector2f size, Vector2f position=Vector2f(0,0));
	static void sendSignal(Layer layer, int id, Node *sender);
	static void sendSignal(int id, Node *sender);
	static void scheduleReload(Node *buffer);

	//Layer control features
	static void pauseLayer(Layer layer, bool pause=true);
	static void hideLayer(Layer layer, bool hidden=true);
	static void globalLayer(Layer layer, bool global=true);

	//Layer read features
	static bool isLayerPaused(Layer layer);
	static bool isLayerHidden(Layer layer);
	static LayerData &getLayerData(Layer layer);
	static int getLayerCount();

	//Utility Functions
	static int loadTexture(std::string filename);
	static Vector2i getTextureSize(sint index);
	static TextureData &getTextureData(sint index);
	static unsigned long long getImGuiTexture(sint texture);
	static Color pickColor(sint texture, Vector2i position);

	//Start engine
	static void startEngine();
	static void stopEngine();
	static bool isRunning();

	//Main loop functions
	static void processEvents();
	static void update(double time);
	static void draw(Vector2f offset, Vector2i size);
	static void drawNode(Node *source);

	//Sokol callback functions
	static void frame(void);
	static void init(void);
	static void cleanup(void);
};

//Functions to be implemented by the game
void initialize();
Color backgroundColor();
std::string *windowTitle();
std::vector<std::string> &textureFiles();
std::vector<std::string> &layerNames();

//Debug tool insertions
void setupDebugTools();
void addDebugTextures();

//Debug ImGui functions
void imguiShowNode(sint id);
void skyrmionImguiMenu();
void skyrmionImgui();
void gameImguiMenu();
void gameImgui();