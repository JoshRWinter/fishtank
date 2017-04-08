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

	long long last_nano_time=0;

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

		// keep track of steps per second
		{
			static int sps=60,last_time;
			int current_time=time(NULL);
			static unsigned long frame=0;
			if(current_time!=last_time){
				last_time=current_time;
				if(sps<55&&frame>200)
					std::cout<<"sps "<<sps<<" -- having trouble keeping up"<<std::endl;
				sps=0;
			}
			else
				++sps;
			++frame;

			long long nano_time;
			do{
				usleep(300);
				get_nano_time(&nano_time);
			}while(nano_time-last_nano_time<16666600);
			last_nano_time=nano_time;
		}
	}

	// clear the clients
	for(std::vector<Client*>::iterator it=client.begin();it!=client.end();++it)
		delete *it;
	client.clear();

	puts("\ngoodbye");
	return 0;
}

void get_nano_time(long long *t){
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC,&ts);
	*t=((long long)ts.tv_sec*(long long)1000000000)+(long long)ts.tv_nsec;
}
