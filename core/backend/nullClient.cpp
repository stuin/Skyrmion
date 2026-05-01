#include "../UpdateList.h"
//#include "nbnetShared.hpp"

void UpdateList::connectServer(std::string ip, int port) {}
void UpdateList::disconnectServer() {}

bool UpdateList::isConnected() {
	return false;
}

int UpdateList::getNetworkId() {
	return 0;
}

bool UpdateList::isNetworkTick() {
	return false;
}

void UpdateList::processNetworking() {}
void UpdateList::processNetworkMessage() {}
void UpdateList::sendNetworkEvent(Event event, bool reliable) {}
void UpdateList::sendNetworkString(std::string data, int code, bool reliable) {}