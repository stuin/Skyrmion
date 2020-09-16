#pragma once

#include <string>
#include <vector>
#include <exception>
#include <bitset>
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
	static Node *(screen)[MAXLAYER];
	static std::bitset<MAXLAYER> alwaysLoadedLayers;
	static std::vector<Node *> deleted;

	//Display variables
	static Node *camera;
	static Node *pointer;
	static sf::View viewPlayer;
	static std::bitset<MAXLAYER> hiddenLayers;

	static bool running;

	//Update loops
	static void update(double time);
	static void draw(sf::RenderWindow &window);
	static void renderingThread(std::string title, sf::VideoMode mode);

public:
	//Manage node lists
	static void addNode(Node *next);
	static void clearLayer(unsigned char layer);

	//Special featurs
	static Node *setCamera(Node *follow, sf::Vector2f size);
	static void setPointer(Node *follow);
	static void alwaysLoadLayer(unsigned char layer);
	static void hideLayer(unsigned char layer, bool hidden=true);

	//Start engine
	static void startEngine(std::string title, sf::VideoMode mode);
};