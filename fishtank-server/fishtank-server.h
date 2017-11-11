#ifndef FISHTANK_SERVER_H
#define FISHTANK_SERVER_H

#include <vector>
#include <cstdint>
#include <iostream>
#include <math.h>

#ifndef _WIN32
#include <unistd.h>
#endif // _WIN32

#include "../jni/fishnet.h"
#include "../jni/network.h"
struct Match;
struct Client;
struct area_bounds;
#include "object.h"

#define onein(n) (randomint(0,n-1)==0)
#define WIN_TIMER 300

#define KILLED_BY_AIRSTRIKE 1
#define KILLED_BY_SHELL 2
#define KILLED_BY_MINE 3
#define KILLED_BY_DECOMPRESSION 4

struct Client{
	Client(int,const area_bounds&,const std::vector<Mine>&,int);
	void kick(Match&);

	net::tcp tcp;
	const std::string connector_address;
	std::string name;
	static int last_id;
	int32_t id;
	int32_t udp_secret;
	net::udp_id udpid;

	// gameplay related
	int colorid;
	Player player;
	int killed_by_id; // killed by client id
	int kill_reason; // KILLED_BY_AIRSTRIKE etc

	struct{
		bool left,right,up,aim_left,aim_right;
		float fire,astrike;
	}input;

	// some stats to keep track of
	struct{
		int join_time;
		int match_victories;
		int victories;
		int deaths;
		int rounds_played;
	}stat;
};

struct area_bounds{
	float left,right,bottom,top;
};

struct ScoreboardEntry;
class Match{
public:
	Match();
	~Match();
	operator bool()const;
	void accept_new_clients();
	void player_summary(const Client &client)const;
	void step();
	void send_data();
	void recv_data();
	void send_chat(const std::string&,const std::string&);
	void send_scoreboard(Client&);
	Client *get_client_by_secret(int32_t);
	int get_client_index(int)const;
	void send_level_config(Client&);
	bool check_win();
	int living_clients()const;
	Client *last_man_standing();
	void ready_next_round();
	void wait_next_step();
	static void scoreboard_sort(std::vector<ScoreboardEntry>&);
	static int get_score(const Client&);

	std::vector<Client*> client_list;
	std::vector<Shell*> shell_list;
	std::vector<Platform> platform_list;
	std::vector<Airstrike*> airstrike_list;
	std::vector<Mine> mine_list;
	net::tcp_server tcp;
	net::udp_server udp;
#ifdef _WIN32
	LARGE_INTEGER last_frame;
#else
	long long last_nano_time;
#endif // _WIN32
	int win_timer;
	bool sent_win_message;
	int round_id;
	int backdrop_index;
	// bounds of the playing area
	area_bounds bounds;
};

struct ScoreboardEntry{
	ScoreboardEntry(const Client *c):client(c),points(Match::get_score(*c)){}
	const Client *client;
	int points; // filled by Match::get_score()
};

void get_nano_time(long long*);
int randomint(int,int);
float zerof(float*,float);
float targetf(float*,float,float);
int32_t sntohl(int32_t);
int32_t shtonl(int32_t);

#endif // FISHTANK_SERVER_H
