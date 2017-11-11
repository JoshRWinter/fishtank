#include <atomic>
#include <signal.h>
#include <time.h>

#include "fishtank-server.h"

std::atomic<bool> run;

int ctrl_c_count=0; // only accessed by signal_handler
#ifdef _WIN32
BOOL WINAPI handler(DWORD signal){
	if(signal==CTRL_C_EVENT){
		run.store(false);
		if(ctrl_c_count++>0)
			exit(1);
		return TRUE;
	}

	return FALSE;
}
#else
void signal_handler(int s){
	switch(s){
	case SIGINT:
	case SIGTERM:
		run.store(false);
		if(ctrl_c_count++>0)
			exit(1);
		break;
	default:
		break;
	}
}
#endif // _WIN32

int main(){
	srand(time(NULL));
	run.store(true);

#ifdef _WIN32
	SetConsoleCtrlHandler(handler, TRUE);
#else
	signal(SIGINT,signal_handler);
	signal(SIGTERM,signal_handler);
	signal(SIGPIPE,signal_handler);
#endif // _WIN32

	Match match;
	if(!match){
		std::cout<<"error: couldn't bind to port "<<TCP_PORT<<" and/or port "<<UDP_PORT<<std::endl;
		std::cout<<"make sure no other server is using ports "<<TCP_PORT<<" (TCP) and "<<UDP_PORT<<" (UDP)"<<std::endl;
		return 1;
	}

	std::cout<<"[ready on tcp:"<<TCP_PORT<<" udp:"<<UDP_PORT<<"]"<<std::endl;

	while(run.load()){
		match.accept_new_clients();

		match.step();

		match.wait_next_step();
	}

	puts("\ngoodbye");
	return 0;
}

#ifdef _WIN32
static LARGE_INTEGER freq, start;
static auto rc=QueryPerformanceFrequency(&freq);
static auto rc2=QueryPerformanceCounter(&start);
void get_nano_time(long long *t){
	const long long per_mill=freq.QuadPart/1'000'000'000;

	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	*t=(counter.QuadPart-start.QuadPart)/per_mill;
}
#else
void get_nano_time(long long *t){
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC,&ts);
	*t=((long long)ts.tv_sec*(long long)1000000000)+(long long)ts.tv_nsec;
}
#endif // _WIN32

int randomint(int low,int high){
	if(low>=high)return low;
	return low+(rand()%(high-low+1));
}

float zerof(float *val,float step){
	if(*val>0.0f){
		*val-=step;
		if(*val<0.0f)*val=0.0f;
	}
	else if(*val<0.0f){
		*val+=step;
		if(*val>0.0f)*val=0.0f;
	}
	return *val;
}

float targetf(float *val,float step,float target){
	if(*val>target){
		*val-=step;
		if(*val<target)*val=target;
	}
	else if(*val<target){
		*val+=step;
		if(*val>target)*val=target;
	}
	return *val;
}

union reorder{
	int32_t i;
	signed char c[4];
};


int32_t sntohl(int32_t i){
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	reorder before;
	reorder after;

	before.i=i;
	after.c[0]=before.c[3];
	after.c[1]=before.c[2];
	after.c[2]=before.c[1];
	after.c[3]=before.c[0];

	return after.i;
#else
	return i
#endif
}

int32_t shtonl(int32_t i){
	return sntohl(i);
}
