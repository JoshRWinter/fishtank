#include <errno.h>
#include "fishtank.h"

Match::Match(){
	quit();
}

Match::~Match(){
	quit();
}

void Match::initialize(const std::string &name){
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
	strncpy(name_tmp,name.c_str(),MSG_LIMIT+1);
	tcp.send(name_tmp,MSG_LIMIT+1);

	// get the udp secret
	int32_t udp_secret_tmp;
	tcp.recv(&udp_secret_tmp,4);
	udp_secret=ntohl(udp_secret_tmp);
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
	tsh.state[CLIENT_STATE_PRESS_DOWN]=htonl(state.input.down.active);
	tsh.state[CLIENT_STATE_PRESS_UP]=htonl(state.input.up.active);
	tsh.state[CLIENT_STATE_PRESS_FIRE]=htonl(state.input.fire.active);
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
			logcat("%s says %s",tctcp.name,tctcp.msg);
			break;
		}
	}

	// collect state updates from the server
	while(udp.peek()>=SIZEOF_TO_CLIENT_HEARTBEAT){
		to_client_heartbeat tch;
		udp.recv(&tch,SIZEOF_TO_CLIENT_HEARTBEAT);

		int i=0;
		for(Player &player:state.player_list){
			player.health=ntohl(tch.state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_HEALTH]);
			player.xv=(int)ntohl(tch.state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_XV])/FLOAT_MULTIPLIER;
			player.yv=(int)ntohl(tch.state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_YV])/FLOAT_MULTIPLIER;
			player.x=(int)ntohl(tch.state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_XPOS])/FLOAT_MULTIPLIER;
			player.y=(int)ntohl(tch.state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_YPOS])/FLOAT_MULTIPLIER;
			player.turret.rot=ntohl(tch.state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_ANGLE])/FLOAT_MULTIPLIER;
			player.colorid=ntohl(tch.state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_COLORID]);
			player.cue_fire=ntohl(tch.state[(i*SERVER_STATE_FIELDS)+SERVER_STATE_FIRE]);

			++i;
		}

	}
}
