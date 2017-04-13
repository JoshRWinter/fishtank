#include <string.h>
#include "fishtank-server.h"

Match::Match(){
	last_nano_time=0;
}
Match::~Match(){
	// clear the clients
	for(std::vector<Client*>::iterator it=client_list.begin();it!=client_list.end();++it)
		delete *it;
	client_list.clear();
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
			socket_tcp tmp=connector_socket;
			uint8_t i=0;
			tmp.send(&i,sizeof(uint8_t));
		}
		else{
			// accept the client
			Client *c=new Client(connector_socket);
			client_list.push_back(c);
			std::cout<<c->name<<" just connected ("<<connector_address<<")"<<std::endl;
			// inform the other clients of the new player
			std::string msg=c->name+" has connected";
			std::cout<<msg<<std::endl;
			send_chat(msg,"server");
		}
	}
}

void Match::step(){
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
				std::cout<<client.name<<" has disconnected."<<std::endl;
				std::string msg=client.name+" has disconnected";
				client.kick();
				delete *it;
				it=client_list.erase(it);

				// inform the other clients of the disconnection
				send_chat(msg,"server");
				continue;
			}

			++it;
		}
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
				std::cout<<client.name<<" says "<<tstcp.msg<<std::endl;
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

		// update the client with the new info
		client->input.left=ntohl(tsh.state[CLIENT_STATE_PRESS_LEFT]);
		client->input.right=ntohl(tsh.state[CLIENT_STATE_PRESS_LEFT]);
		client->input.down=ntohl(tsh.state[CLIENT_STATE_PRESS_LEFT]);
		client->input.up=ntohl(tsh.state[CLIENT_STATE_PRESS_LEFT]);
		client->input.aim_left=ntohl(tsh.state[CLIENT_STATE_PRESS_LEFT]);
		client->input.aim_right=ntohl(tsh.state[CLIENT_STATE_PRESS_LEFT]);
		client->input.fire=ntohl(tsh.state[CLIENT_STATE_PRESS_FIRE])/FLOAT_MULTIPLIER;
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
}

Client *Match::get_client_by_secret(int32_t s){
	for(Client *c:client_list){
		if(c->udp_secret==s)
			return c;
	}

	return NULL;
}

void Match::wait_next_step(){
	// keep track of steps per second
	static int sps=60,last_time;
	int current_time=time(NULL);
	static unsigned long frame=0;
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
