#include "Event.h"

class NetworkList {
private:
	static bool networkInitialized;
	static bool networkClient;
	static bool networkConnected;
	static int networkId;
	static unsigned int networkTimer;

public:
	static void processNetworking();
	static void processNetworkMessage();

	//Networking client functions
	static void connectServer(std::string ip, int port);
	static void disconnectServer();
	static bool isConnected();
	static int getNetworkId();
	static bool isNetworkTick();
	static void sendNetworkEvent(Event event, bool reliable=false);
	static void sendNetworkString(std::string data, int code=0, bool reliable=true);
};

//Networking functions to be implemented by the game
//Define NETWORK_STRING to enable
void recieveNetworkString(std::string data, int code);