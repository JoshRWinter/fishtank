#include "fishtank-server.h"

Client::Client(int s):tcp(s){
	// accept the client
	uint8_t accepted=1;
	tcp.send(&accepted,sizeof(uint8_t));

	// get the client's name
	char string[MSG_LIMIT+1];
	tcp.recv(string,MSG_LIMIT+1);
	string[MSG_LIMIT]=0; // carefully
	name=string;
}

Client::~Client(){
	kick();
}

void Client::kick(){
	tcp.close();
}
