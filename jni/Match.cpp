#include "fishtank.h"

Match::Match(){
	quit();
}

Match::~Match(){
	quit();
}

void Match::initialize(const std::string &name,socket_tcp &s){
	// setup the tcp socket
	tcp=s;
	// prevent s destructor from closing its internal socket
	s.disable();
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
	char tmp[MSG_LIMIT+1];
	strcpy(tmp,name.c_str());
	tcp.send(name.c_str(),MSG_LIMIT+1);

	// get the udp secret
	int32_t udp_secret_tmp;
	tcp.recv(&udp_secret_tmp,4);
	udp_secret=ntohl(udp_secret_tmp);
}

bool Match::connected(){
	return !tcp.error();
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
}
