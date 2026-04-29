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


//Event operators
bool operator==(const Event &first, const Event &second) {
    return first.type == second.type && first.down == second.down && first.code == second.code &&
        first.x == second.x && first.y == second.y;
}
bool operator!=(const Event &first, const Event &second) {
    return !(first == second);
}

std::ostream& operator<<(std::ostream& os, const Event &e) {
    return os << std::to_string(e.type) << ':' << std::to_string(e.down) << ':' << std::to_string(e.code) <<
        "(" << std::to_string(e.x) << ',' << std::to_string(e.y) << ") ";
}