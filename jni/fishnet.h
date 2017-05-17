#ifndef FISHNET_H
#define FISHNET_H

#include <cstdint>

#define TCP_PORT 28856
#define UDP_PORT 28857

#define MAX_PLAYERS 6
#define MSG_LIMIT 45

#define FLOAT_MULTIPLIER 10000.0f

// message from client to server tcp
#define TYPE_CHAT 0
#define TYPE_NEW_PLAYER 1
#define TYPE_DELETE_PLAYER 2
#define TYPE_HEARTBEAT 3
#define TYPE_NEW_LEVEL 4
#define SIZEOF_TO_SERVER_TCP (1+MSG_LIMIT+1)
struct to_server_tcp{
	uint8_t type;
	uint8_t msg[MSG_LIMIT+1];
};

// absolute state update heartbeat from client to server udp
#define CLIENT_STATE_PRESS_LEFT 0
#define CLIENT_STATE_PRESS_RIGHT 1
#define CLIENT_STATE_PRESS_UP 2
#define CLIENT_STATE_PRESS_FIRE 3
#define CLIENT_STATE_PRESS_ASTRIKE 4
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

// absolute state update from server to client udp
#define SERVER_STATE_GLOBAL_PLATFORMS 0
#define SERVER_STATE_GLOBAL_PLATFORMS_EXT 1
#define SERVER_STATE_GLOBAL_AIRSTRIKE_NEW 2
#define SERVER_STATE_GLOBAL_AIRSTRIKE_XPOS 3
#define SERVER_STATE_GLOBAL_AIRSTRIKE_XV 4
#define SERVER_STATE_GLOBAL_FIELDS 5
#define SERVER_STATE_HEALTH 0
#define SERVER_STATE_XV 1
#define SERVER_STATE_YV 2
#define SERVER_STATE_XPOS 3
#define SERVER_STATE_YPOS 4
#define SERVER_STATE_BEACON_XV 5
#define SERVER_STATE_BEACON_YV 6
#define SERVER_STATE_BEACON_XPOS 7
#define SERVER_STATE_BEACON_YPOS 8
#define SERVER_STATE_BEACON_ROT 9
#define SERVER_STATE_ANGLE 10
#define SERVER_STATE_COLORID 11
#define SERVER_STATE_FIRE 12
#define SERVER_STATE_ID 13
#define SERVER_STATE_PLATFORMS 14
#define SERVER_STATE_FIELDS 15
#define SIZEOF_TO_CLIENT_HEARTBEAT ((4*MAX_PLAYERS*SERVER_STATE_FIELDS)+(4*SERVER_STATE_GLOBAL_FIELDS))
struct to_client_heartbeat{
	int32_t state[SERVER_STATE_GLOBAL_FIELDS+(MAX_PLAYERS*SERVER_STATE_FIELDS)];
};

#define COLOR_RED 1
#define COLOR_BLUE 2
#define COLOR_CYAN 3
#define COLOR_GREEN 4
#define COLOR_PURPLE 5

#define VIEW_LEFT -8.0f
#define VIEW_RIGHT 8.0f
#define VIEW_BOTTOM 4.5f
#define VIEW_TOP -4.5f

#define ARTILLERY_DOWNWARD_VELOCITY 0.385f
#define ARTILLERY_SIZE 0.35f
#define ARTILLERY_Y -20.0f

#define BEACON_WIDTH 0.7f
#define BEACON_HEIGHT 0.2125f
#define PLAYER_WIDTH 1.5f
#define PLAYER_HEIGHT 1.0f
#define TURRET_WIDTH 1.5f
#define TURRET_HEIGHT 0.2f

#define SHELL_WIDTH 0.5f
#define SHELL_HEIGHT 0.5f

#define FLOOR 4.0f
#define LEFT -15.0f
#define RIGHT 40.0f
#define GRAVITY 0.008f
#define PLAYER_GRAVITY (GRAVITY/2.0f)

#define PLATFORM_COUNT 64
#define PLATFORM_WIDTH 3.75f
#define PLATFORM_HEIGHT 0.65f

#endif // FISHNET_H
