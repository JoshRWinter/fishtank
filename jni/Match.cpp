#include <cstdint>
#include "fishtank.h"

Match::Match(){
	tcp=NULL;
	quit();
}

Match::~Match(){
	quit();
}

void Match::initialize(const std::string &name,socket_tcp *tcp_socket){
	connected=true;
	tcp=tcp_socket;

	// send the name
	uint8_t name_length=name.length();
	tcp->send(&name_length,sizeof(uint8_t));
	tcp->send(name.c_str(),name_length);
}

bool Match::running(){
	return connected;
}

void Match::quit(){
	connected=false;

	delete tcp;
	tcp=NULL;
}

void send_data(const State &state){
}

void recv_data(State &state){
}
