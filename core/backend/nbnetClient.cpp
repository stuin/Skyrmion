#include "../UpdateList.h"

#define NBNET_IMPL
#include "nbnetShared.hpp"

bool UpdateList::networkInitialized = false;
bool UpdateList::networkClient = false;
bool UpdateList::networkConnected = false;
int UpdateList::networkId = 0;
unsigned int UpdateList::networkTimer = 0;

void UpdateList::connectServer(std::string ip, int port) {
	std::cout << "NETWORK: Creating client for server " << ip << ":" << port << "\n";
	if(!networkInitialized) {
		#ifdef PLATFORM_WEB
			NBN_WebRTC_Register(); // Register the WebRTC driver
		#else
			NBN_UDP_Register(); // Register the UDP driver
		#endif
		networkInitialized = true;
	}

	if(NBN_GameClient_StartEx(PROTOCOL_NAME, ip.c_str(), port, NULL, 0) >= 0) {
		networkClient = true;

		NBN_GameClient_RegisterMessage(MESSAGE_EVENT,
			(NBN_MessageBuilder)Event_Create,
			(NBN_MessageDestructor)Event_Destroy,
			(NBN_MessageSerializer)Event_Serialize);
		NBN_GameClient_RegisterMessage(MESSAGE_STRING,
			(NBN_MessageBuilder)NetworkString_Create,
			(NBN_MessageDestructor)NetworkString_Destroy,
			(NBN_MessageSerializer)NetworkString_Serialize);

		std::cout << "NETWORK: Started client\n";
	}
}
void UpdateList::disconnectServer() {
	std::cout << "NETWORK: Destroying client\n";
	networkConnected = false;
	networkClient = false;
	NBN_GameClient_Stop();
}

bool UpdateList::isConnected() {
	return networkInitialized && networkClient && networkConnected;
}

int UpdateList::getNetworkId() {
	return networkId;
}

bool UpdateList::isNetworkTick() {
	return networkTimer % 20 == 0;
}

void UpdateList::processNetworking() {
	if(!networkInitialized || !networkClient)
		return;
	networkTimer++;

	int ev;
	while((ev = NBN_GameClient_Poll()) != NBN_NO_EVENT) {
		if(ev < 0) {
			Log("ERROR", "NETWORK: An error occured while polling client events");
			break;
		}
		//std::cout << "NETWORK: Event: ";

		switch(ev) {
			case NBN_CONNECTED:
				std::cout << "NETWORK: Connected to server\n";
				networkConnected = true;
				break;
			case NBN_DISCONNECTED:
				std::cout << "NETWORK: Disconnected from server\n";
				networkConnected = false;
				networkId = 0;
				UpdateList::queueEvent(Event(EVENT_NETWORK_CONNECT_SERVER, true, 0));
				break;
			case NBN_MESSAGE_RECEIVED:
				processNetworkMessage();
				break;
		}
	}

	if(NBN_GameClient_SendPackets() < 0)
		Log("ERROR", "NETWORK: An occured while flushing the send queue");
}

void UpdateList::processNetworkMessage() {
	//Get info about the received message
	NBN_MessageInfo msg_info = NBN_GameClient_GetMessageInfo();

	if(msg_info.type == MESSAGE_EVENT) {
		//Convert message to Event
		Event *msg = (Event*)msg_info.data;
		Event event = *msg;
		UpdateList::queueEvent(event);
		std::cout << "NETWORK: Received event from server: " << event << "\n";
		Event_Destroy(msg);

		//Respond to connection event
		if(event.type == EVENT_NETWORK_CONNECT_SERVER && networkConnected)
			networkId = event.code;
	} else if(msg_info.type == MESSAGE_STRING) {
		//Convert message to String
		NetworkString *msg = (NetworkString*)msg_info.data;
		msg->data[msg->length-1] = '\0';
		std::string s = std::string(msg->data);
		recieveNetworkString(s, msg->code);
		std::cout << "NETWORK: Received string from server: " << msg->code << s << "\n";
		NetworkString_Destroy(msg);
	}
}

void UpdateList::sendNetworkEvent(Event event, bool reliable) {
	//Create the message
	Event *msg = Event_Create();
	if(msg == NULL)
		return;

	//Copy event data
	*msg = event;
	std::cout << "NETWORK: Sent event to server: " << event << "\n";

	//Send it to the server
	if(reliable) {
		if(NBN_GameClient_SendReliableMessage(MESSAGE_EVENT, msg) < 0)
			Log("ERROR", "NETWORK: Error occured while sending event");
	} else {
		if(NBN_GameClient_SendUnreliableMessage(MESSAGE_EVENT, msg) < 0)
			Log("ERROR", "NETWORK: Error occured while sending event");
	}
}

void UpdateList::sendNetworkString(std::string data, int code, bool reliable) {
	unsigned int length = data.length()+1;
	if(length > NETWORK_STRING_LENGTH) {
		std::cout << "NETWORK: String too long to send\n";
		length = NETWORK_STRING_LENGTH;
	}
	const char *c = data.c_str();

	//Create the string message
	NetworkString *msg = NetworkString_Create();
	if(msg == NULL)
		return;

	//Fill message with string
	msg->code = code;
	msg->length = length;
	memcpy(msg->data, c, length);
	msg->data[length-1] = '\0';

	std::cout << "NETWORK: Sent string to server: " << msg->data << "\n";

	//Send it to the server
	if(reliable) {
		if(NBN_GameClient_SendReliableMessage(MESSAGE_STRING, msg) < 0)
			Log("ERROR", "NETWORK: Error occured while sending string");
	} else {
		if(NBN_GameClient_SendUnreliableMessage(MESSAGE_STRING, msg) < 0)
			Log("ERROR", "NETWORK: Error occured while sending string");
	}
}