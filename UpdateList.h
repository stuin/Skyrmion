#pragma once

#include <string>
#include <vector>
#include <deque>
#include <bitset>
#include <atomic>
#include <unordered_map>
#include <exception>
#include <thread>
#include <chrono>

#include "Node.h"

/*
 * Created by Stuart Irwin on 4/13/2019.
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

	//Display variables
	static Node *camera;
	static sf::View viewPlayer;
	static WindowSize windowSize;
	static std::bitset<MAXLAYER> hiddenLayers;

	static Layer max;
	static bool running;

	//Update loops
	static void update(double time);
	static void draw(sf::RenderWindow &window);
	static void renderingThread(std::string title);

public:
	//Manage node lists
	static void addNode(Node *next);
	static void clearLayer(Layer layer);
	static void addListener(Node *item, sf::Event::EventType type);

	//Special features
	static Node *setCamera(Node *follow, sf::Vector2f size, sf::Vector2f position=sf::Vector2f(0,0));
	static void staticLayer(Layer layer, bool _static=true);
	static void pauseLayer(Layer layer, bool pause=true);
	static void hideLayer(Layer layer, bool hidden=true);

	//Utility Functions
	static void loadTexture(sf::Texture *texture, std::string filename);
	static sf::Texture *getTexture(int index);

	//Start engine
	static void startEngine(std::string title, Layer max);
	static void stopEngine();
};