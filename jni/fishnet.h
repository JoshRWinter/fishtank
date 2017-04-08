#define TCP_PORT 28856
#define UDP_PORT 28857

#define MAX_PLAYERS 6
#define MSG_LIMIT 45

#define FLOAT_MULTIPLIER 10000

// to server structures
#define TYPE_CHAT 0
#define TYPE_NEW_PLAYER 1
#define TYPE_DELETE_PLAYER 2
struct to_server_tcp{
	uint8_t type;
	uint8_t msg[MSG_LIMIT+1];
};

// udp
#define STATE_PRESS_LEFT 0
#define STATE_PRESS_RIGHT 1
#define STATE_PRESS_DOWN 2
#define STATE_PRESS_UP 3
#define STATE_PRESS_FIRE 4
#define STATE_PRESS_FIREPOWER 5
#define STATE_PRESS_AIMLEFT 6
#define STATE_PRESS_AIMRIGHT 7
#define STATE_ID 8
struct to_server_heartbeat{
	uint8_t state[9]; 
};

struct to_client_tcp{
	uint8_t type;
	uint8_t msg[MSG_LIMIT+1];
	uint8_t name[MSG_LIMIT+1];
	uint8_t id;
};

struct to_client_heartbeat{
	int32_t state[MAX_PLAYERS*9];
};
