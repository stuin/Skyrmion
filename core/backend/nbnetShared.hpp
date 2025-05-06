#include <stdarg.h>

void Log(const char *level, const char *fmt, ...) {
    va_list args;

    if(level[0] == 'T')
        return;

    va_start(args, fmt);

    printf("[%s] ", level);
    vprintf(fmt, args);
    printf("\n");

    va_end(args);
}

// nbnet logging, use printf logging
#define NBN_LogInfo(...) Log("INFO", __VA_ARGS__)
#define NBN_LogError(...) Log("ERROR", __VA_ARGS__)
#define NBN_LogWarning(...) Log("WARNING", __VA_ARGS__)
#define NBN_LogDebug(...) Log("DEBUG", __VA_ARGS__)
#define NBN_LogTrace(...) Log("TRACE", __VA_ARGS__)

#include "../Event.h"

#include "../../include/nbnet/nbnet.h"

#ifdef PLATFORM_WEB
#include "../../include/nbnet/net_drivers/webrtc.h"
#else
#include "../../include/nbnet/net_drivers/udp.h"
//#include "../../include/nbnet/net_drivers/webrtc_c.h"
#endif

#define PROTOCOL_NAME "SkyrmionBase"
#define SERVER_FULL_CODE 42

#define NETWORK_STRING_LENGTH 255
#define FLOAT_MIN -10000.0f
#define FLOAT_MAX 10000.0f

enum NETWORK_MESSAGE {
    MESSAGE_EVENT,
    MESSAGE_STRING
};
struct NetworkString {
	int code = 0;
	unsigned int length = 0;
	char data[NETWORK_STRING_LENGTH];
};

Event *Event_Create(void) {
    return (Event *) malloc(sizeof(Event));
}

void Event_Destroy(Event *msg) {
    free(msg);
}

int Event_Serialize(Event *msg, NBN_Stream *stream) {
    NBN_SerializeInt(stream, msg->type, INT_MIN, INT_MAX);
    NBN_SerializeBool(stream, msg->down);
    NBN_SerializeInt(stream, msg->code, INT_MIN, INT_MAX);
    NBN_SerializeFloat(stream, msg->x, FLOAT_MIN, FLOAT_MAX, 3);
    NBN_SerializeFloat(stream, msg->y, FLOAT_MIN, FLOAT_MAX, 3);

    return 0;
}

NetworkString *NetworkString_Create(void) {
    return (NetworkString *) malloc(sizeof(NetworkString));
}

void NetworkString_Destroy(NetworkString *msg) {
    free(msg);
}

int NetworkString_Serialize(NetworkString *msg, NBN_Stream *stream) {
	NBN_SerializeInt(stream, msg->code, INT_MIN, INT_MAX);
    NBN_SerializeUInt(stream, msg->length, 0, NETWORK_STRING_LENGTH);
    NBN_SerializeBytes(stream, msg->data, msg->length);

    return 0;
}

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
std::ostream& operator<<(std::ostream& os, const NetworkString &s) {
    return os << std::to_string(s.code) << ':' << s.data << "\n";
}

/*
//Event struct handling
Event *Event_Create(void);
void Event_Destroy(Event *msg);
int Event_Serialize(Event *msg, NBN_Stream *stream);

//NetworkString struct handling
NetworkString *NetworkString_Create(void);
void NetworkString_Destroy(NetworkString *msg);
int NetworkString_Serialize(NetworkString *msg, NBN_Stream *stream);
*/