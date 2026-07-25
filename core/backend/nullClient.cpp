#include "../NetworkList.h"
//#include "nbnetShared.hpp"

void NetworkList::connectServer(std::string ip, int port) {}
void NetworkList::disconnectServer() {}

bool NetworkList::isConnected() {
	return false;
}

int NetworkList::getNetworkId() {
	return 0;
}

bool NetworkList::isNetworkTick() {
	return false;
}

void NetworkList::processNetworking() {}
void NetworkList::processNetworkMessage() {}
void NetworkList::sendNetworkEvent(Event event, bool reliable) {}
void NetworkList::sendNetworkString(std::string data, int code, bool reliable) {}