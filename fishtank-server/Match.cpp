#include <string.h>
#include "fishtank-server.h"

Match::Match(){
	last_nano_time=0;
	win_timer=WIN_TIMER;
	round_id=0;
}
Match::~Match(){
	// clear the clients
	for(std::vector<Client*>::iterator it=client_list.begin();it!=client_list.end();++it)
		delete *it;
	client_list.clear();

	// clear the shell list
	for(std::vector<Shell*>::iterator it=shell_list.begin();it!=shell_list.end();++it)
		delete *it;
	shell_list.clear();

	// clear the airstrike list
	for(Airstrike *as:airstrike_list)
		delete as;
	airstrike_list.clear();
}

bool Match::setup(){
	return tcp.bind(TCP_PORT)&&udp.bind(UDP_PORT);
}

void Match::accept_new_clients(){
	// accept new clients
	std::string connector_address;
	int connector_socket=tcp.accept(connector_address);
	if(connector_socket!=-1){
		if(client_list.size()==MAX_PLAYERS){
			// have to kick the client
			std::cout<<"rejected "<<connector_address<<", too many players!"<<std::endl;
			socket_tcp tmp(connector_socket,connector_address);
			uint8_t i=0;
			tmp.send(&i,sizeof(uint8_t));
		}
		else{
			// if this is the first client, reset the level config
			if(client_list.size()==0){
				ready_next_round();
			}

			// accept the client
			Client *c=new Client(connector_socket,connector_address,bounds,mine_list);
			client_list.push_back(c);

			// send the level configuration to this client
			send_level_config(*c);

			std::cout<<c->name<<" just connected ("<<connector_address<<")"<<std::endl;
			// inform the other clients of the new player
			std::string msg=c->name+" has connected";
			send_chat(msg,"server");
			// current time
			c->stat.join_time=time(NULL);
		}
	}
}

void Match::player_summary(const Client &client)const{
	const int current=time(NULL);
	std::cout<<client.name<<" summary:"<<std::endl;
	std::cout<<" -- rounds played: "<<client.stat.rounds_played<<std::endl;
	std::cout<<" -- time played: "<<((current-client.stat.join_time)/60)<<"m "<<((current-client.stat.join_time)%60)<<"s"<<std::endl;
	std::cout<<" -- match victories: "<<client.stat.match_victories<<std::endl;
	std::cout<<" -- one-on-one victories: "<<client.stat.victories<<std::endl;
	std::cout<<" -- deaths: "<<client.stat.deaths<<std::endl;
}

// execute one step
void Match::step(){
	// process shells
	Shell::process(*this);
	// process platforms
	Platform::process(platform_list);
	// process airstrikes
	Airstrike::process(*this);
	// process mines
	Mine::process(*this);
	// process players
	Player::process(*this);

	// check for a win
	if(client_list.size()>0&&shell_list.size()==0&&airstrike_list.size()==0){
		if(win_timer<WIN_TIMER){
			--win_timer;
			if(win_timer<1&&sent_win_message){
				// initialize the next round
				ready_next_round();
			}
		}
		else{
			if(check_win())
				--win_timer;
		}
	}

	recv_data();
	send_data();
}

void Match::send_data(){
	// send out a heartbeat to all clients to see if they're still connected
	if(onein(100)){
		for(std::vector<Client*>::iterator it=client_list.begin();it!=client_list.end();){
			Client &client=**it;
			to_client_tcp heartbeat;

			memset(&heartbeat,0,sizeof(to_client_tcp));
			heartbeat.type=TYPE_HEARTBEAT;
			client.tcp.send(&heartbeat.type,sizeof(heartbeat.type));
			client.tcp.send(&heartbeat.msg,sizeof(heartbeat.msg));
			client.tcp.send(&heartbeat.name,sizeof(heartbeat.name));
			if(client.tcp.error()){
				// kick
				std::string ip;
				client.tcp.get_name(ip);
				std::cout<<client.name<<" has disconnected ("<<ip<<")"<<std::endl;
				std::string msg=client.name+" has disconnected";
				client.kick(*this);
				// round summary
				player_summary(client);

				// free up
				delete *it;
				it=client_list.erase(it);

				// inform the other clients of the disconnection
				send_chat(msg,"server");
				continue;
			}

			++it;
		}
	}

	// data about a possible new artillery
	float arty_xpos=0.0f;
	float arty_xv=0.0f;
	bool arty_new=false;
	for(Airstrike *strike:airstrike_list){
		bool out=false; // break control, since everybody's so scared of goto
		for(Artillery *arty:strike->arty_list){
			if(!arty->processed){
				arty->processed=true;

				arty_new=true;
				arty_xpos=arty->x;
				arty_xv=arty->xv;

				out=true;
				break;
			}
		}
		if(out)
			break;
	}

	// send data to all connected clients
	to_client_heartbeat tch;
	int32_t *server_state=tch.state+SERVER_STATE_GLOBAL_FIELDS;
	memset(&tch,0,sizeof(to_client_heartbeat));
	// SERVER_STATE_GLOBAL_INDEX is set just before sending to client
	tch.state[SERVER_STATE_GLOBAL_ROUND_ID]=htonl(round_id);
	tch.state[SERVER_STATE_GLOBAL_PLATFORMS]=htonl(Platform::platform_status[0]);
	tch.state[SERVER_STATE_GLOBAL_PLATFORMS_EXT]=htonl(Platform::platform_status[1]);
	tch.state[SERVER_STATE_GLOBAL_MINES]=htonl(Mine::mine_status);
	tch.state[SERVER_STATE_GLOBAL_AIRSTRIKE_NEW]=htonl(arty_new);
	tch.state[SERVER_STATE_GLOBAL_AIRSTRIKE_XPOS]=shtonl(arty_xpos*FLOAT_MULTIPLIER);
	tch.state[SERVER_STATE_GLOBAL_AIRSTRIKE_XV]=shtonl(arty_xv*FLOAT_MULTIPLIER);
	int i=0;

	// initialize tch with state data
	for(Client *c:client_list){
		Client &client=*c;

		server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_HEALTH]=htonl(client.player.health);
		server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_XV]=shtonl(client.player.xv*FLOAT_MULTIPLIER);
		server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_YV]=shtonl(client.player.yv*FLOAT_MULTIPLIER);
		server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_XPOS]=shtonl((client.player.x-client.player.xv)*FLOAT_MULTIPLIER);
		server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_YPOS]=shtonl((client.player.y-client.player.yv)*FLOAT_MULTIPLIER);
		server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_BEACON_XV]=shtonl(client.player.beacon.xv*FLOAT_MULTIPLIER);
		server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_BEACON_YV]=shtonl(client.player.beacon.yv*FLOAT_MULTIPLIER);
		server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_BEACON_XPOS]=shtonl((client.player.beacon.x-client.player.beacon.xv)*FLOAT_MULTIPLIER);
		server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_BEACON_YPOS]=shtonl((client.player.beacon.y-client.player.beacon.yv)*FLOAT_MULTIPLIER);
		server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_BEACON_ROT]=shtonl(client.player.beacon.rot*FLOAT_MULTIPLIER);
		server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_ANGLE]=shtonl(client.player.angle*FLOAT_MULTIPLIER);
		server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_COLORID]=htonl(client.colorid);
		server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_FIRE]=shtonl(client.input.fire*FLOAT_MULTIPLIER);

		++i;
	}

	// dispatch state data
	i=0;
	for(Client *c:client_list){
		Client &client=*c;
		if(!client.udpid.initialized)
			continue;

		tch.state[SERVER_STATE_GLOBAL_INDEX]=htonl(i);
		udp.send(&tch,SIZEOF_TO_CLIENT_HEARTBEAT,client.udpid);

		++i;
	}
}

void Match::recv_data(){
	// collect data from tcp
	for(std::vector<Client*>::iterator it=client_list.begin();it!=client_list.end();++it){
		Client &client=**it;

		if(client.tcp.peek()>=SIZEOF_TO_SERVER_TCP){
			to_server_tcp tstcp;

			client.tcp.recv(&tstcp.type,sizeof(tstcp.type));
			client.tcp.recv(&tstcp.msg,sizeof(tstcp.msg));
			tstcp.msg[MSG_LIMIT]=0; // carefully

			switch(tstcp.type){
			case TYPE_HEARTBEAT:
				// ignore
				break;
			case TYPE_CHAT:
				send_chat((char*)tstcp.msg,client.name);
				break;
			case TYPE_SCOREBOARD:
				send_scoreboard(client);
				break;
			case TYPE_SPECTATED_NAME:
				{
					to_client_tcp tctcp;
					memset(&tctcp,0,sizeof(tctcp));
					uint32_t index;
					client.tcp.recv(&index,sizeof(index));
					index=ntohl(index);
					if(index<client_list.size())
						strncpy((char*)tctcp.name,client_list[index]->name.c_str(),MSG_LIMIT+1);
					else
						tctcp.name[0]=0;
					tctcp.type=TYPE_SPECTATED_NAME;
					client.tcp.send(&tctcp.type,sizeof(tctcp.type));
					client.tcp.send(tctcp.msg,sizeof(tctcp.msg));
					client.tcp.send(tctcp.name,sizeof(tctcp.name));
				}
				break;
			}
		}
	}

	// recv client heartbeats
	while(udp.peek()>=SIZEOF_TO_SERVER_HEARTBEAT){
		to_server_heartbeat tsh;
		memset(&tsh,0,sizeof(to_server_heartbeat));

		udp_id id;
		udp.recv(&tsh.state,SIZEOF_TO_SERVER_HEARTBEAT,id);

		// figure out which player sent this state update
		Client *client=get_client_by_secret(ntohl(tsh.state[CLIENT_STATE_UDP_SECRET]));
		if(client==NULL){
			std::cout<<"unrecognized udp id"<<std::endl;
			continue;
		}
		if(!client->udpid.initialized)
			client->udpid=id;

		// update the client with the new info
		client->input.left=ntohl(tsh.state[CLIENT_STATE_PRESS_LEFT]);
		client->input.right=ntohl(tsh.state[CLIENT_STATE_PRESS_RIGHT]);
		client->input.up=ntohl(tsh.state[CLIENT_STATE_PRESS_UP]);
		client->input.aim_left=ntohl(tsh.state[CLIENT_STATE_PRESS_AIMLEFT]);
		client->input.aim_right=ntohl(tsh.state[CLIENT_STATE_PRESS_AIMRIGHT]);
		if(client->input.fire==0.0f)
			client->input.fire=(int)ntohl(tsh.state[CLIENT_STATE_PRESS_FIRE])/FLOAT_MULTIPLIER;
		if(client->input.astrike==0.0f)
			client->input.astrike=(int)ntohl(tsh.state[CLIENT_STATE_PRESS_ASTRIKE])/FLOAT_MULTIPLIER;
	}
}

// send a chat message to everyone including the speaker
void Match::send_chat(const std::string &msg,const std::string &from){
	to_client_tcp tctcp;
	memset(&tctcp,0,sizeof(to_client_tcp));

	tctcp.type=TYPE_CHAT;
	strncpy((char*)tctcp.msg,msg.c_str(),MSG_LIMIT+1);
	strncpy((char*)tctcp.name,from.c_str(),MSG_LIMIT+1);

	for(std::vector<Client*>::iterator it=client_list.begin();it!=client_list.end();++it){
		Client &client=**it;

		client.tcp.send(&tctcp.type,sizeof(tctcp.type));
		client.tcp.send(&tctcp.msg,sizeof(tctcp.msg));
		client.tcp.send(&tctcp.name,sizeof(tctcp.name));
	}

	// display the chat on stdout
	if(from=="server")
		std::cout<<"chat\033[36m[\033[31mserver: \033[36m";
	else
		std::cout<<"chat\033[36m["<<from<<": ";
	std::cout<<msg<<"]\033[0m"<<std::endl;
}

Client *Match::get_client_by_secret(int32_t s){
	for(Client *c:client_list){
		if(c->udp_secret==s)
			return c;
	}

	return NULL;
}

int Match::get_client_index(int id)const{
	int index=0;
	for(const Client *c:client_list){
		if(c->id==id)
			return index;
		++index;
	}
}

void Match::send_level_config(Client &client){
	// send round id
	uint32_t round_id_tmp=htonl(round_id);
	client.tcp.send(&round_id_tmp,sizeof(round_id_tmp));
	// send backdrop index
	uint32_t backdrop_index_tmp=htonl(backdrop_index);
	client.tcp.send(&backdrop_index_tmp,sizeof(backdrop_index_tmp));
	// send platform
	for(const Platform &platform:platform_list){
		int32_t horiz=platform.horiz;
		int32_t x=(platform.x+(platform.w/2.0f))*FLOAT_MULTIPLIER;
		int32_t y=(platform.y+(platform.h/2.0f))*FLOAT_MULTIPLIER;
		int32_t health=platform.health;
		uint32_t seed=platform.seed;

		horiz=htonl(horiz);
		x=htonl(x);
		y=htonl(y);
		health=htonl(health);
		seed=htonl(seed);

		client.tcp.send(&horiz,sizeof(horiz));
		client.tcp.send(&x,sizeof(x));
		client.tcp.send(&y,sizeof(y));
		client.tcp.send(&health,sizeof(health));
		client.tcp.send(&seed,sizeof(seed));
	}

	// mines
	uint32_t count=htonl(mine_list.size());
	client.tcp.send(&count,sizeof(count));
	for(const Mine &mine:mine_list){
		uint32_t platform_index=htonl(mine.platform_index);
		uint32_t armed=htonl(mine.armed);

		client.tcp.send(&platform_index,sizeof(platform_index));
		client.tcp.send(&armed,sizeof(armed));
	}
}

int Match::get_score(const Client &c){
	int kills=c.stat.victories-c.stat.match_victories; // but not victories

	return (c.stat.match_victories*2)+kills;
}

// descending order
void Match::scoreboard_sort(std::vector<ScoreboardEntry> &entries){
	// the allmighty insertion sort
	for(int i=1;i<entries.size();++i){
		int j=i;
		while(j>0&&entries[j-1].points<entries[j].points){
			ScoreboardEntry tmp=entries[j];
			entries[j]=entries[j-1];
			entries[j-1]=tmp;

			--j;
		}
	}
}

void Match::send_scoreboard(Client &client){
	to_client_tcp tctcp;
	memset(&tctcp,0,sizeof(tctcp));
	tctcp.type=TYPE_SCOREBOARD;
	client.tcp.send(&tctcp.type,sizeof(tctcp.type));
	client.tcp.send(&tctcp.msg,sizeof(tctcp.msg));
	client.tcp.send(&tctcp.name,sizeof(tctcp.name));

	// populate the scoreboard entries
	std::vector<ScoreboardEntry> entries;
	for(const Client *c:client_list){
		ScoreboardEntry e(c);

		entries.push_back(e);
	}

	// sort by points
	Match::scoreboard_sort(entries);

	uint32_t count=htonl(client_list.size());
	client.tcp.send(&count,sizeof(count));
	for(const ScoreboardEntry &e:entries){
		// send name
		char name[MSG_LIMIT+1];
		strncpy(name,e.client->name.c_str(),MSG_LIMIT+1);
		client.tcp.send(name,MSG_LIMIT+1);

		// send (boolean) currently dead
		uint32_t dead=e.client->player.health<1;
		client.tcp.send(&dead,sizeof(dead));

		// send match victories
		uint32_t mv=htonl(e.client->stat.match_victories);
		client.tcp.send(&mv,sizeof(mv));

		// send one-on-one victories
		uint32_t ooo=htonl(e.client->stat.victories);
		client.tcp.send(&ooo,sizeof(ooo));

		// send deaths
		uint32_t d=htonl(e.client->stat.deaths);
		client.tcp.send(&d,sizeof(d));

		// send points
		uint32_t p=htonl(e.points);
		client.tcp.send(&p,sizeof(p));
	}
}

bool Match::check_win(){
	int alive=living_clients();
	std::string win_message;

	if(alive==1&&client_list.size()>1){
		Client *winner=last_man_standing();
		if(winner==NULL)
			return true;
		win_message=winner->name+" wins!";
		send_chat(win_message,"server");
		sent_win_message=true;
		if(win_timer<100)
			win_timer+=150;
		++winner->stat.match_victories;
		return true;
	}
	else if(alive==0){
		win_message="Nobody wins! Hurray!";
		send_chat(win_message,"server");
		sent_win_message=true;
		if(win_timer<100)
			win_timer+=150;
		return true;
	}

	return false;
}

// how many players are alive
int Match::living_clients()const{
	int alive=0;
	for(const Client *c:client_list)
		if(c->player.health>0)
			++alive;

	return alive;
}

// gets name of first living client found in the list
// check Match::living_clients()==1 before calling, otherwise this function is useless
Client *Match::last_man_standing(){
	for(Client *c:client_list){
		if(c->player.health>0){
			return c;
		}
	}
	return NULL;
}

// construct a new level and send it
void Match::ready_next_round(){
	sent_win_message=false;
	++round_id;
	backdrop_index=randomint(0,3);
	Platform::create_all(*this);
	Mine::create_all(*this);
	Platform::update(platform_list);
	Mine::update(mine_list);
	to_client_tcp tctcp;
	memset(&tctcp,0,sizeof(tctcp));
	tctcp.type=TYPE_NEW_LEVEL;
	for(Client *client:client_list){
		client->tcp.send(&tctcp.type,sizeof(tctcp.type));
		client->tcp.send(&tctcp.msg,sizeof(tctcp.msg));
		client->tcp.send(&tctcp.name,sizeof(tctcp.name));
		send_level_config(*client);
		// increment rounds_played
		++client->stat.rounds_played;
	}
	// reset
	win_timer=WIN_TIMER;
	for(Client *client:client_list)
		client->player.reset(bounds,mine_list);
}

void Match::wait_next_step(){
	// keep track of steps per second
	static int sps=60,last_time;
	int current_time=time(NULL);
	static unsigned long frame=0;

	// sleep longer if no one is connected
	if(client_list.size()==0){
		sleep(1);
		last_time=time(NULL);
		frame=0;
		return;
	}

	if(current_time!=last_time){
		last_time=current_time;
		// server is falling behind
		if(sps<55&&frame>200)
			std::cout<<"sps "<<sps<<" -- having trouble keeping up"<<std::endl;
		sps=0;
	}
	else
		++sps;
	++frame;

 	// block until time to do next step
	long long nano_time;
	do{
		sched_yield();
		usleep(300);
		get_nano_time(&nano_time);
	}while(nano_time-last_nano_time<16666600);
	last_nano_time=nano_time;
}
