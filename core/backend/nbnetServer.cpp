#include <iostream>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>



#define NBNET_IMPL
#include "nbnetShared.hpp"

#define MAX_CLIENTS 4
#define EVENTS_PER_CLIENT 20
#define STRINGS_PER_CLIENT 10

struct Client {
	//Underlying nbnet connection handle, used to send messages to that particular client
	NBN_ConnectionHandle client_handle;
	unsigned int id;

	//Client state queues
	Event events[EVENTS_PER_CLIENT];
	NetworkString strings[STRINGS_PER_CLIENT];

	//Client state indexes
	int recievedEvents = 0;
	int sentEvents = 0;
	int recievedStrings = 0;
	int sentStrings = 0;
};

static Client *clients[MAX_CLIENTS] = {NULL};
static unsigned int client_count = 0;

static int HandleNewConnection(void) {
	Log("INFO", "New connection");

	// If the server is full
	if(client_count == MAX_CLIENTS) {
		// Reject the connection (send a SERVER_FULL_CODE code to the client)
		Log("INFO", "Connection rejected");
		NBN_GameServer_RejectIncomingConnectionWithCode(SERVER_FULL_CODE);
		return 0;
	}

	NBN_ConnectionHandle client_handle;
	client_handle = NBN_GameServer_GetIncomingConnection();

	Client *client = NULL;
	for(unsigned int i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i] == NULL) {
			client = (Client*)malloc(sizeof(Client));
			clients[i] = client;
			client->id = i+1;
			break;
		}
	}
	assert(client != NULL);

	client->client_handle = client_handle; // Store the nbnet connection ID

	// Accept the connection
	NBN_GameServer_AcceptIncomingConnection();

	Event *msg = Event_Create();
	msg->type = EVENT_NETWORK_CONNECT_SERVER;
	msg->down = false;
	msg->code = client->id;
	NBN_GameServer_SendReliableMessageTo(client_handle, MESSAGE_EVENT, msg);

	Log("INFO", "Connection accepted (ID: %d / %d)", client_handle, client->id);

	client_count++;
	return 0;
}

static void HandleClientDisconnection() {
	NBN_ConnectionHandle client_handle = NBN_GameServer_GetDisconnectedClient(); // Get the disconnected client

	Log("INFO", "Client has disconnected (id: %d)", client_handle);
	int i;
	//Delete client from list
	for(i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i] && clients[i]->client_handle == client_handle) {
			free(clients[i]);
			clients[i] = NULL;
			break;
		}
	}
	client_count--;

	//Notify other clients
	for(int j = 0; j < MAX_CLIENTS; j++) {
		if(clients[j]) {
			Event *msg = Event_Create();
			msg->type = EVENT_NETWORK_CONNECT_CLIENT;
			msg->down = true;
			msg->code = i+1;
			NBN_GameServer_SendReliableMessageTo(clients[j]->client_handle, MESSAGE_EVENT, msg);
		}
	}

}

static Client *FindClientById(uint32_t client_id) {
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i] && clients[i]->client_handle == client_id)
			return clients[i];
	}
	return NULL;
}

static void HandleReceivedMessage(void) {
	// Fetch info about the last received message
	NBN_MessageInfo msg_info = NBN_GameServer_GetMessageInfo();

	// Find the client that sent the message
	Client *sender = FindClientById(msg_info.sender);
	assert(sender != NULL);

	switch(msg_info.type) {
		case MESSAGE_EVENT:
			sender->events[sender->recievedEvents] = *(Event*)msg_info.data;
			std::cout << "Recieved event " << sender->recievedEvents << " from client " << sender->client_handle << ": " << *(Event*)msg_info.data << "\n";
			sender->recievedEvents = (sender->recievedEvents + 1) % EVENTS_PER_CLIENT;
			Event_Destroy((Event*)msg_info.data);
			break;
		case MESSAGE_STRING:
			sender->strings[sender->recievedStrings] = *(NetworkString*)msg_info.data;
			std::cout << "Recieved string "<< sender->recievedStrings << " from client " << sender->client_handle << ": " << *(NetworkString*)msg_info.data << "\n";
			sender->recievedStrings = (sender->recievedStrings + 1) % STRINGS_PER_CLIENT;
			NetworkString_Destroy((NetworkString*)msg_info.data);
			break;
	}
}

static int HandleGameServerEvent(int ev) {
	switch(ev) {
		case NBN_NEW_CONNECTION:
			// A new client has requested a connection
			if(HandleNewConnection() < 0)
				return -1;
			break;
		case NBN_CLIENT_DISCONNECTED:
			// A previously connected client has disconnected
			HandleClientDisconnection();
			break;
		case NBN_CLIENT_MESSAGE_RECEIVED:
			// A message from a client has been received
			HandleReceivedMessage();
			break;
	}
	return 0;
}

static int BroadcastGameState(void) {
	//std::cout << "Broadcasting data\n";
	for(int i = 0; i < MAX_CLIENTS; i++) {
		Client *sender = clients[i];
		if(sender == NULL) continue;

		//Sending Events
		while(sender->sentEvents != sender->recievedEvents) {
			for(int j = 0; j < MAX_CLIENTS; j++) {
				if(clients[j] != NULL && i != j) {
					Event *msg = Event_Create();
					*msg = sender->events[sender->sentEvents];

					if(msg->type <= EVENT_NETWORK_CONNECT_CLIENT)
						NBN_GameServer_SendReliableMessageTo(clients[j]->client_handle, MESSAGE_EVENT, msg);
					else
						NBN_GameServer_SendUnreliableMessageTo(clients[j]->client_handle, MESSAGE_EVENT, msg);
					//std::cout << "Sent event " << sender->sentEvents << " from " << i+1 << " to " << j+1 << ": " << sender->events[sender->sentEvents] << "\n";
				}
			}
			sender->sentEvents = (sender->sentEvents + 1) % EVENTS_PER_CLIENT;
		}

		//Sending Strings
		while(sender->sentStrings != sender->recievedStrings) {
			sender->sentStrings = (sender->sentStrings + 1) % EVENTS_PER_CLIENT;
			for(int j = 0; j < MAX_CLIENTS; j++) {
				if(clients[j] != NULL && i != j) {
					NetworkString *msg = NetworkString_Create();
					*msg = sender->strings[sender->sentStrings];

					NBN_GameServer_SendReliableMessageTo(clients[j]->client_handle, MESSAGE_STRING, msg);
				}
			}
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
	NBN_UDP_Register();

	int port = 1234;

	if(NBN_GameServer_StartEx(PROTOCOL_NAME, port) < 0) {
		Log("ERROR", "Game server failed to start. Exit");
		return 1;
	}

	NBN_GameServer_RegisterMessage(MESSAGE_EVENT,
		(NBN_MessageBuilder)Event_Create,
		(NBN_MessageDestructor)Event_Destroy,
		(NBN_MessageSerializer)Event_Serialize);
	NBN_GameServer_RegisterMessage(MESSAGE_STRING,
		(NBN_MessageBuilder)NetworkString_Create,
		(NBN_MessageDestructor)NetworkString_Destroy,
		(NBN_MessageSerializer)NetworkString_Serialize);

	float tick_dt = 0.01; // Tick delta time
	while(true) {
		int ev;

		// Poll for server events
		while((ev = NBN_GameServer_Poll()) != NBN_NO_EVENT) {
			if(ev < 0) {
				Log("ERROR", "An occured while polling network events. Exit");
				break;
			}

			if(HandleGameServerEvent(ev) < 0)
				break;
		}

		// Broadcast latest game state
		if(BroadcastGameState() < 0) {
			Log("ERROR", "An occured while broadcasting game states. Exit");
			break;
		}

		// Pack all enqueued messages as packets and send them
		if(NBN_GameServer_SendPackets() < 0) {
			Log("ERROR", "An occured while flushing the send queue. Exit");
			break;
		}

		//NBN_GameServerStats stats = NBN_GameServer_GetStats();
		//if(stats.upload_bandwidth != 0 || stats.download_bandwidth != 0)
		//	Log("TRACE", "Upload: %f Bps | Download: %f Bps", stats.upload_bandwidth, stats.download_bandwidth);

		// Cap the simulation rate to TICK_RATE ticks per second (just like for the client)
		//long nanos = tick_dt * 1e9;
		//struct timespec t = {.tv_sec = nanos / 999999999, .tv_nsec = nanos % 999999999};
		//nanosleep(&t, &t);
	}

	// Stop the server
	NBN_GameServer_Stop();
	return 0;
}