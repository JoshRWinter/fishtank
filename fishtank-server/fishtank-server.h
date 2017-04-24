#ifndef FISHTANK_SERVER_H
#define FISHTANK_SERVER_H

#include <vector>
#include <cstdint>
#include <unistd.h>
#include <iostream>
#include "../jni/fishnet.h"
#include "../jni/network.h"
struct Match;
struct Client;
#include "object.h"

#define onein(n) (randomint(0,n-1)==0)
#define WIN_TIMER 150

struct Client{
	Client(int,const std::string&);
	void kick(Match&);

	socket_tcp tcp;
	std::string name;
	static int last_id;
	int32_t id;
	int32_t udp_secret;
	udp_id udpid;

	// gameplay related
	int colorid;
	Player player;

	struct{
		bool left,right,up,aim_left,aim_right;
		float fire;
	}input;
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
	void send_chat(const std::string&,const std::string&);
	Client *get_client_by_secret(int32_t);
	void send_level_config(Client&);
	bool check_win();
	void wait_next_step();

	std::vector<Client*> client_list;
	std::vector<Shell*> shell_list;
	std::vector<Platform> platform_list;
	socket_tcp_server tcp;
	socket_udp_server udp;
	long long last_nano_time;
	int win_timer;
};

void get_nano_time(long long*);
int randomint(int,int);
float zerof(float*,float);
float targetf(float*,float,float);

#endif // FISHTANK_SERVER_H
