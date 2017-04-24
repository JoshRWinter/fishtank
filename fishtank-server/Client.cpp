#include "fishtank-server.h"

int Client::last_id=0;
Client::Client(int s,const std::string &addr):tcp(s,addr){
	// accept the client
	uint8_t accepted=1;
	tcp.send(&accepted,sizeof(uint8_t));

	// get the client's name
	char string[MSG_LIMIT+1];
	tcp.recv(string,MSG_LIMIT+1);
	string[MSG_LIMIT]=0; // carefully
	name=string;

	// generate and send the udp secret
	// weird i know
	udp_secret=rand()%1000000;
	int32_t tmp=htonl(udp_secret);
	tcp.send(&tmp,4);

	// send the clients id
	Client::last_id++;
	id=Client::last_id;
	int id_tmp=htonl(last_id);
	tcp.send(&id_tmp,sizeof(id_tmp));

	colorid=4;
	input.left=false;
	input.right=false;
	input.up=false;
	input.aim_left=false;
	input.aim_right=false;
	input.fire=0.0f;
}

void Client::kick(Match &match){
	tcp.close();

	// delete all shells associated with this client
	for(std::vector<Shell*>::iterator it=match.shell_list.begin();it!=match.shell_list.end();){
		if(&(*it)->owner==this){
			delete *it;
			it=match.shell_list.erase(it);
			continue;
		}

		++it;
	}
}
