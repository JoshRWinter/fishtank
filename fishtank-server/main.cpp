#include <atomic>
#include <signal.h>
#include <time.h>

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
	srand(time(NULL));
	run.store(true);
	signal(SIGINT,signal_handler);
	signal(SIGTERM,signal_handler);
	signal(SIGPIPE,signal_handler);

	Match match;
	if(!match.setup()){
		std::cout<<"error: couldn't bind to port "<<TCP_PORT<<" and/or port "<<UDP_PORT<<std::endl;
		std::cout<<"make sure no other server is using ports "<<TCP_PORT<<" (TCP) and "<<UDP_PORT<<" (UDP)"<<std::endl;
		return 1;
	}

	while(run.load()){
		match.accept_new_clients();

		match.step();

		match.wait_next_step();
	}

	puts("\ngoodbye");
	return 0;
}

void get_nano_time(long long *t){
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC,&ts);
	*t=((long long)ts.tv_sec*(long long)1000000000)+(long long)ts.tv_nsec;
}

int randomint(int low,int high){
	if(low>=high)return low;
	return low+(rand()%(high-low+1));
}
