#include <errno.h>
#include "fishtank.h"

Match::Match(){
	my_index=0;
}

Match::~Match(){
	quit();
}

void Match::initialize(State &state){
	// setup the udp socket
	std::string address;
	tcp.get_name(address);
	if(!udp.setup(address,UDP_PORT)){
		logcat("error: could not setup udp socket");
		tcp.close();
		udp.close();
		return;
	}

	// send the name
	char name_tmp[MSG_LIMIT+1];
	strncpy(name_tmp,state.name.c_str(),MSG_LIMIT+1);
	tcp.send(name_tmp,MSG_LIMIT+1);

	// send the colorid
	uint32_t colorid_tmp=htonl(state.colorid);
	tcp.send(&colorid_tmp,sizeof(colorid_tmp));

	// get the udp secret
	int32_t udp_secret_tmp;
	tcp.recv(&udp_secret_tmp,4);
	udp_secret=ntohl(udp_secret_tmp);

	// get the id
	uint32_t id_tmp;
	tcp.recv(&id_tmp,sizeof(id_tmp));
	id=ntohl(id_tmp);

	// get the level configuration
	get_level_config(state);
}

bool Match::connected(){
	return !tcp.error();
}

socket_tcp &Match::get_tcp(){
	return tcp;
}

void Match::quit(){
	tcp.close();
	udp.close();
}

void Match::send_data(const State &state){
	// send a tcp heartbeat to see if the connection is still alive
	if(onein(30)){
		to_server_tcp heartbeat;
		heartbeat.type=TYPE_HEARTBEAT;
		tcp.send(&heartbeat.type,sizeof(heartbeat.type));
		tcp.send(&heartbeat.msg,sizeof(heartbeat.msg));
		if(tcp.error()){
			quit();
			return;
		}
	}

	// send udp state update heartbeat
	to_server_heartbeat tsh;
	memset(&tsh,0,sizeof(to_server_heartbeat));

	tsh.state[CLIENT_STATE_PRESS_LEFT]=htonl(state.input.left.active);
	tsh.state[CLIENT_STATE_PRESS_RIGHT]=htonl(state.input.right.active);
	tsh.state[CLIENT_STATE_PRESS_UP]=htonl(state.input.up_r.active||state.input.up_l.active);
	tsh.state[CLIENT_STATE_PRESS_FIRE]=htonl(state.final_firepower*FLOAT_MULTIPLIER);
	tsh.state[CLIENT_STATE_PRESS_ASTRIKE]=htonl(state.final_strokepower*FLOAT_MULTIPLIER);
	tsh.state[CLIENT_STATE_PRESS_AIMLEFT]=htonl(state.input.aim_left.active);
	tsh.state[CLIENT_STATE_PRESS_AIMRIGHT]=htonl(state.input.aim_right.active);
	tsh.state[CLIENT_STATE_UDP_SECRET]=htonl(udp_secret);

	udp.send(&tsh.state,SIZEOF_TO_SERVER_HEARTBEAT);
}

void Match::recv_data(State &state){
	// collect tcp info
	if(tcp.peek()>=SIZEOF_TO_CLIENT_TCP){
		to_client_tcp tctcp;

		tcp.recv(&tctcp.type,sizeof(tctcp.type));
		tcp.recv(&tctcp.msg,sizeof(tctcp.msg));
		tcp.recv(&tctcp.name,sizeof(tctcp.name));

		// carefully
		tctcp.msg[MSG_LIMIT]=0;
		tctcp.name[MSG_LIMIT]=0;

		switch(tctcp.type){
		case TYPE_HEARTBEAT:
			// ignore;
			break;
		case TYPE_CHAT:
			if(!strcmp((const char*)tctcp.name,"server")){ // message from the server
				ServerMessage sm((const char*)tctcp.name,(const char*)tctcp.msg);
				state.announcement.push_back(sm);
			}
			else{
				ChatMessage cm((const char*)tctcp.name,(const char*)tctcp.msg);
				state.chat.push_back(cm);
				state.timer_chatpane=TIMER_CHATPANE;
			}
			break;
		case TYPE_NEW_LEVEL:
			get_level_config(state);
			for(ParticlePlayer *p:state.particle_player_list)
				delete p;
			state.particle_player_list.clear();
			break;
		case TYPE_SCOREBOARD:
			get_scoreboard(state.scoreboard);
			break;
		}
	}

	// collect state updates from the server
	uint32_t platform_status[2];
	uint32_t mine_status;
	while(udp.peek()>=SIZEOF_TO_CLIENT_HEARTBEAT){
		to_client_heartbeat tch;
		udp.recv(&tch,SIZEOF_TO_CLIENT_HEARTBEAT);
		int32_t *server_state=tch.state+SERVER_STATE_GLOBAL_FIELDS;

		platform_status[0]=ntohl(tch.state[SERVER_STATE_GLOBAL_PLATFORMS]);
		platform_status[1]=ntohl(tch.state[SERVER_STATE_GLOBAL_PLATFORMS_EXT]);
		mine_status=ntohl(tch.state[SERVER_STATE_GLOBAL_MINES]);
		bool arty_new=ntohl(tch.state[SERVER_STATE_GLOBAL_AIRSTRIKE_NEW]);
		float arty_xpos=(int)ntohl(tch.state[SERVER_STATE_GLOBAL_AIRSTRIKE_XPOS])/FLOAT_MULTIPLIER;
		float arty_xv=(int)ntohl(tch.state[SERVER_STATE_GLOBAL_AIRSTRIKE_XV])/FLOAT_MULTIPLIER;

		int i=0;
		for(Player &player:state.player_list){
			int before_health=player.health;

			player.health=(int)ntohl(server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_HEALTH]);
			player.xv=(int)ntohl(server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_XV])/FLOAT_MULTIPLIER;
			player.yv=(int)ntohl(server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_YV])/FLOAT_MULTIPLIER;
			player.x=(int)ntohl(server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_XPOS])/FLOAT_MULTIPLIER;
			player.y=(int)ntohl(server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_YPOS])/FLOAT_MULTIPLIER;
			player.beacon.xv=(int)ntohl(server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_BEACON_XV])/FLOAT_MULTIPLIER;
			player.beacon.yv=(int)ntohl(server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_BEACON_YV])/FLOAT_MULTIPLIER;
			player.beacon.x=(int)ntohl(server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_BEACON_XPOS])/FLOAT_MULTIPLIER;
			player.beacon.y=(int)ntohl(server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_BEACON_YPOS])/FLOAT_MULTIPLIER;
			player.beacon.rot=(int)ntohl(server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_BEACON_ROT])/FLOAT_MULTIPLIER;
			player.turret.rot=(int)ntohl(server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_ANGLE])/FLOAT_MULTIPLIER;
			player.colorid=ntohl(server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_COLORID]);
			if(player.cue_fire==0.0f)
				player.cue_fire=(int)ntohl(server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_FIRE])/FLOAT_MULTIPLIER;
			int cid=ntohl(server_state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_ID]);
			if(id==cid)
				my_index=i;
			if(before_health>0&&player.health<1&&player.colorid!=0){
				ParticlePlayer::spawn(state,player);
			}

			++i;
		}

		// update the platforms
		for(i=0;i<32;++i){
			bool before=state.platform_list[i].active;
			state.platform_list[i].active=((platform_status[0]>>i)&1)==1;
			if(before&&!state.platform_list[i].active){
				// spawn particle effects
				ParticlePlatform::spawn_destroy_platform(state,state.platform_list[i]);
			}
		}
		for(i=0;i<32;++i){
			bool before=state.platform_list[i+32].active;
			state.platform_list[i+32].active=((platform_status[1]>>i)&1)==1;
			if(before&&!state.platform_list[i+32].active){
				// spawn particle effects
				ParticlePlatform::spawn_destroy_platform(state,state.platform_list[i+32]);
			}
		}

		// update the mines
		for(int i=0;i<state.mine_list.size();++i){
			bool before=state.mine_list[i].armed;
			state.mine_list[i].armed=((mine_status>>i)&1)==1;
			if(before&&!state.mine_list[i].armed){
				ParticleBubble::spawn(state,state.mine_list[i]);
			}
		}

		// maybe new artillery
		if(arty_new)
			state.arty_list.push_back(new Artillery(arty_xpos,arty_xv));
	}
}

void Match::send_chat(const std::string &message){
	to_server_tcp tstcp;

	tstcp.type=TYPE_CHAT;
	if(message.length()>MSG_LIMIT)
		return;
	strcpy((char*)tstcp.msg,message.c_str());

	tcp.send(&tstcp,sizeof(tstcp));
}

void Match::get_level_config(State &state){
	// get platforms
	state.platform_list.clear();
	for(int i=0;i<PLATFORM_COUNT;++i){
		int32_t horiz;
		int32_t x;
		int32_t y;
		int32_t health;
		uint32_t seed;

		tcp.recv(&horiz,sizeof(horiz));
		tcp.recv(&x,sizeof(x));
		tcp.recv(&y,sizeof(y));
		tcp.recv(&health,sizeof(health));
		tcp.recv(&seed,sizeof(seed));

		horiz=ntohl(horiz);
		x=ntohl(x);
		y=ntohl(y);
		health=ntohl(health);
		seed=ntohl(seed);

		Platform p(health>0,horiz,x/FLOAT_MULTIPLIER,y/FLOAT_MULTIPLIER,seed);

		state.platform_list.push_back(p);
	}

	// get mines
	state.mine_list.clear();
	uint32_t count;
	tcp.recv(&count,sizeof(count));
	count=ntohl(count);
	for(int i=0;i<count;++i){
		uint32_t platform_index;
		uint32_t armed;
		tcp.recv(&platform_index,sizeof(platform_index));
		tcp.recv(&armed,sizeof(armed));
		platform_index=ntohl(platform_index);
		armed=ntohl(armed);

		Mine mine(state.platform_list,platform_index,armed);
		state.mine_list.push_back(mine);
	}
}

void Match::request_scoreboard(){
	// send the request
	to_server_tcp tstcp;
	memset(&tstcp,0,sizeof(tstcp));
	tstcp.type=TYPE_SCOREBOARD;
	tcp.send(&tstcp.type,sizeof(tstcp.type));
	tcp.send(&tstcp.msg,sizeof(tstcp.msg));
}

void Match::get_scoreboard(std::vector<stat> &stat_list){
	int bytestr=0;
	// get the count
	uint32_t count;
	tcp.recv(&count,sizeof(count));
	count=ntohl(count);

	for(int i=0;i<count;++i){
		stat s;

		// get the name
		char name[MSG_LIMIT+1];
		tcp.recv(name,MSG_LIMIT+1);
		name[MSG_LIMIT]=0; // carefully

		// get the match victories
		uint32_t mv;
		tcp.recv(&mv,sizeof(mv));
		mv=ntohl(mv);

		// get the one-on-one-victories
		uint32_t ooo;
		tcp.recv(&ooo,sizeof(ooo));
		ooo=ntohl(ooo);

		// get the deaths
		uint32_t d;
		tcp.recv(&d,sizeof(d));
		d=ntohl(d);

		s.name=name;
		s.match_victories=mv;
		s.victories=ooo;
		s.deaths=d;

		stat_list.push_back(s);
	}
}
