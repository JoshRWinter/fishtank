#include <cstdint>

#define TCP_PORT 28856
#define UDP_PORT 28857

#define MAX_PLAYERS 6
#define MSG_LIMIT 45

#define FLOAT_MULTIPLIER 10000

// message from client to server tcp
#define TYPE_CHAT 0
#define TYPE_NEW_PLAYER 1
#define TYPE_DELETE_PLAYER 2
#define TYPE_HEARTBEAT 3
#define SIZEOF_TO_SERVER_TCP (1+MSG_LIMIT+1)
struct to_server_tcp{
	uint8_t type;
	uint8_t msg[MSG_LIMIT+1];
};

// absolute state update heartbeat from client to server udp
#define STATE_PRESS_LEFT 0
#define STATE_PRESS_RIGHT 1
#define STATE_PRESS_DOWN 2
#define STATE_PRESS_UP 3
#define STATE_PRESS_FIRE 4
#define STATE_PRESS_AIMLEFT 5
#define STATE_PRESS_AIMRIGHT 6
#define STATE_HEALTH 7
#define STATE_COLORID 8
#define STATE_UDP_SECRET 9
#define SIZEOF_TO_SERVER_HEARTBEAT (4*10)
struct to_server_heartbeat{
	int32_t state[10];
};

// message from server to client tcp
#define SIZEOF_TO_CLIENT_TCP (1+(MSG_LIMIT+1)+(MSG_LIMIT+1)+1)
struct to_client_tcp{
	uint8_t type;
	uint8_t msg[MSG_LIMIT+1];
	uint8_t name[MSG_LIMIT+1];
};

// absolute state update from client to server udp
#define SIZEOF_TO_CLIENT_HEARTBEAT (4*MAX_PLAYERS*10)
struct to_client_heartbeat{
	int32_t state[MAX_PLAYERS*10];
};
