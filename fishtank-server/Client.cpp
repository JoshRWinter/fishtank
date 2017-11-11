#include "fishtank-server.h"

int Client::last_id=0;
Client::Client(int s,const area_bounds &bounds,const std::vector<Mine> &mine_list,int player_count):tcp(s),connector_address(tcp.get_name()),player(bounds,mine_list,player_count){
	// accept the client
	uint8_t accepted=1;
	tcp.send_block(&accepted,sizeof(uint8_t));

	// get the client's name
	char string[MSG_LIMIT+1];
	tcp.recv_block(string,MSG_LIMIT+1);
	string[MSG_LIMIT]=0; // carefully
	name=string;
	// prevent the name "server"
	if(name=="server")
		name="server ";

	// get the colorid
	uint32_t colorid_tmp;
	tcp.recv_block(&colorid_tmp,sizeof(colorid_tmp));
	colorid=ntohl(colorid_tmp);

	// generate and send the udp secret
	// weird i know
	udp_secret=rand()%1000000;
	int32_t tmp=htonl(udp_secret);
	tcp.send_block(&tmp,4);

	// send the clients id
	Client::last_id++;
	id=Client::last_id;
	uint32_t id_tmp=htonl(last_id);
	tcp.send_block(&id_tmp,sizeof(id_tmp));

	// send number of players
	uint32_t count_tmp=htonl(player_count);
	tcp.send_block(&count_tmp,sizeof(count_tmp));

	killed_by_id=0;
	kill_reason=0;

	input.left=false;
	input.right=false;
	input.up=false;
	input.aim_left=false;
	input.aim_right=false;
	input.fire=0.0f;
	input.astrike=0.0f;

	stat.join_time=0;
	stat.match_victories=0;
	stat.victories=0;
	stat.deaths=0;
	stat.rounds_played=0;
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

	// delete all airstrikes associated with this client
	for(std::vector<Airstrike*>::iterator it=match.airstrike_list.begin();it!=match.airstrike_list.end();){
		if(&(*it)->client==this){
			delete *it;
			it=match.airstrike_list.erase(it);
			continue;
		}
	}
}
