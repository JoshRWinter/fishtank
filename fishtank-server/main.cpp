#include <iostream>
#include <atomic>
#include <signal.h>
#include <unistd.h>
#include <cstdint>
#include <vector>

#include "fishtank-server.h"

std::atomic<bool> run;

void signal_handler(int s){
	switch(s){
	case SIGINT:
	case SIGTERM:
		run.store(false);
		break;
	default:
		break;
	}
}

int main(){
	run.store(true);
	signal(SIGINT,signal_handler);
	signal(SIGTERM,signal_handler);
	signal(SIGPIPE,signal_handler);

	socket_tcp_server server;
	if(!server.bind(TCP_PORT)){
		printf("error: couldn't bind to port %d.",TCP_PORT);
		return 1;
	}

	int client_count=0;
	std::vector<Client*> client;

	while(run.load()){
		// accept new clients
		std::string connector_address;
		int connector_socket=server.accept(connector_address);
		if(connector_socket!=-1){
			if(client_count==MAX_PLAYERS){
				// have to kick the client
				std::cout<<"rejected "<<connector_address<<", too many players!"<<std::endl;
				socket_tcp tcp=connector_socket;
				uint8_t i=0;
				tcp.send(&i,sizeof(int8_t));
			}
			else{
				// accept the client
				Client *c=new Client(connector_socket);
				client.push_back(c);
				++client_count;
				std::cout<<c->name<<" just connected ("<<connector_address<<")"<<std::endl;
			}
		}

		usleep(100);
	}

	// clear the clients
	for(std::vector<Client*>::iterator it=client.begin();it!=client.end();++it)
		delete *it;
	client.clear();

	puts("\ngoodbye");
	return 0;
}
