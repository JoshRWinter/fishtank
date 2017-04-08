#include <cstdint>
#include "fishtank.h"

Match::Match(){
	quit();
}

Match::~Match(){
	quit();
}

void Match::initialize(const std::string &name,socket_tcp &s){
	connected=true;

	// setup the socket
	tcp=s;
	s.disable();

	// send the name
	uint8_t name_length=name.length();
	tcp.send(&name_length,sizeof(uint8_t));
	tcp.send(name.c_str(),name_length);
}

bool Match::running(){
	return connected;
}

void Match::quit(){
	tcp.close();
	connected=false;
}

void send_data(const State &state){
}

void recv_data(State &state){
}
