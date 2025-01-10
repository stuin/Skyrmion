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

/*
 * Manages list of nodes through update cycle
 */

class UpdateList {
private:
	//Node management
	static Node *screen[MAXLAYER];
	static std::bitset<MAXLAYER> staticLayers;
	static std::bitset<MAXLAYER> pausedLayers;
	static std::vector<Node *> deleted;

	//Window event system
	static std::atomic_int event_count;
	static std::deque<sf::Event> event_queue;
	static std::unordered_map<sf::Event::EventType, std::vector<Node *>> listeners;

	//Viewport variables
	static Node *camera;
	static sf::View viewPlayer;
	static WindowSize windowSize;

	//Display special cases
	static std::bitset<MAXLAYER> hiddenLayers;
	static std::vector<Node *> reloadBuffer;
	static std::vector<sf::Texture *> textureSet;

	static Layer max;
	static bool running;

	static void renderingThread(std::string title);

public:
	//Manage node lists
	static void addNode(Node *next);
	static void addNodes(std::vector<Node *> nodes);
	static Node *getNode(Layer layer);
	static void clearLayer(Layer layer);
	static void addListener(Node *item, sf::Event::EventType type);

	//Special node features
	static Node *setCamera(Node *follow, sf::Vector2f size, sf::Vector2f position=sf::Vector2f(0,0));
	static void sendSignal(Layer layer, int id, Node *sender);
	static void sendSignal(int id, Node *sender);
	static void scheduleReload(Node *buffer);

	//Layer control features
	static void staticLayer(Layer layer, bool _static=true);
	static void pauseLayer(Layer layer, bool pause=true);
	static void hideLayer(Layer layer, bool hidden=true);

	//Layer read features
	static bool isLayerStatic(Layer layer);
	static bool isLayerPaused(Layer layer);
	static bool isLayerHidden(Layer layer);
	static int getMaxLayer();

	//Utility Functions
	static sf::Texture *loadTexture(sf::Texture *texture, std::string filename, sint index=0);
	static sf::Texture *getTexture(sint index);

	//Start engine
	static void startEngine(std::string title);
	static void stopEngine();
	static bool isRunning();

	//Main loop functions
	static void processEvents();
	static void update(double time);
	static void draw(sf::RenderTarget &window, sf::Vector2f offset=sf::Vector2f(0,0));
};