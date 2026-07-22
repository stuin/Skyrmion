#include "../AudioList.h"
//#include "nbnetShared.hpp"

void AudioList::connectServer(std::string ip, int port) {}
void AudioList::disconnectServer() {}

bool AudioList::isConnected() {
	return false;
}

int AudioList::getNetworkId() {
	return 0;
}

bool AudioList::isNetworkTick() {
	return false;
}

void AudioList::processNetworking() {}
void AudioList::processNetworkMessage() {}
void AudioList::sendNetworkEvent(Event event, bool reliable) {}
void AudioList::sendNetworkString(std::string data, int code, bool reliable) {}