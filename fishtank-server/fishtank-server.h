#include <vector>
#include <cstdint>
#include <unistd.h>
#include <iostream>
#include "../jni/fishnet.h"
#include "../jni/network.h"

#define onein(n) (randomint(0,n-1)==0)

struct Client{
	Client(int);
	~Client();
	void kick();

	socket_tcp tcp;
	std::string name;
};

class Match{
public:
	Match();
	~Match();
	bool setup();
	void accept_new_clients();
	void step();
	void send_data();
	void recv_data();
	void wait_next_step();

	std::vector<Client*> client_list;
	int client_count;
	socket_tcp_server tcp;
	socket_udp_server udp;
	long long last_nano_time;
};

void get_nano_time(long long*);
int randomint(int,int);
