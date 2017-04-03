#include "fishtank-server.h"

Client::Client(int s):tcp(s){
	// accept the client
	uint8_t accepted=1;
	tcp.send(&accepted,sizeof(uint8_t));

	// get the client's name
	char *string;
	uint8_t name_length;
	tcp.recv(&name_length,sizeof(uint8_t));
	string=new char[name_length+1];
	string[name_length]=0; // terminating character
	tcp.recv(string,name_length);
	name=string;
	delete[] string;
	if(name.length()>MSG_LIMIT)
		name.resize(MSG_LIMIT);
}

Client::~Client(){
	kick();
}

void Client::kick(){
	tcp.close();
}
