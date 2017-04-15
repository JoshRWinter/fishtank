#include <cstdint>

#define TCP_PORT 28856
#define UDP_PORT 28857

#define MAX_PLAYERS 6
#define MSG_LIMIT 45

#define FLOAT_MULTIPLIER 10000.0

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
#define CLIENT_STATE_PRESS_LEFT 0
#define CLIENT_STATE_PRESS_RIGHT 1
#define CLIENT_STATE_PRESS_DOWN 2
#define CLIENT_STATE_PRESS_UP 3
#define CLIENT_STATE_PRESS_FIRE 4
#define CLIENT_STATE_PRESS_AIMLEFT 5
#define CLIENT_STATE_PRESS_AIMRIGHT 6
#define CLIENT_STATE_UDP_SECRET 7
#define SIZEOF_TO_SERVER_HEARTBEAT (4*8)
struct to_server_heartbeat{
	int32_t state[8];
};

// message from server to client tcp
#define SIZEOF_TO_CLIENT_TCP (1+(MSG_LIMIT+1)+(MSG_LIMIT+1))
struct to_client_tcp{
	uint8_t type;
	uint8_t msg[MSG_LIMIT+1];
	uint8_t name[MSG_LIMIT+1];
};

// absolute state update from client to server udp
#define SERVER_STATE_HEALTH 0
#define SERVER_STATE_XV 1
#define SERVER_STATE_YV 2
#define SERVER_STATE_XPOS 3
#define SERVER_STATE_YPOS 4
#define SERVER_STATE_ANGLE 5
#define SERVER_STATE_COLORID 6
#define SERVER_STATE_FIRE 7
#define SERVER_STATE_FIELDS 8
#define SIZEOF_TO_CLIENT_HEARTBEAT (4*MAX_PLAYERS*SERVER_STATE_FIELDS)
struct to_client_heartbeat{
	int32_t state[MAX_PLAYERS*SERVER_STATE_FIELDS];
};

#define PLAYER_WIDTH 1.5f
#define PLAYER_HEIGHT 1.0f
#define TURRET_WIDTH 1.5f
#define TURRET_HEIGHT 0.2f

#define FLOOR 4.0f
#define GRAVITY 0.001f
