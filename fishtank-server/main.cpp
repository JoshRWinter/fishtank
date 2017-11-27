#include <atomic>
#include <signal.h>
#include <time.h>
#include <fstream>

#ifndef _WIN32
#include <wordexp.h>
#endif // _WIN32

#include "fishtank-server.h"

std::atomic<bool> run;

static std::string register_master();
static void send_heartbeat(const Match &match);
static net::tcp master;
static bool registered = false;

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

	// signal handlers
#ifdef _WIN32
	SetConsoleCtrlHandler(handler, TRUE);
#else
	signal(SIGINT,signal_handler);
	signal(SIGTERM,signal_handler);
	signal(SIGPIPE,signal_handler);
#endif // _WIN32

	// register on the master server
	const std::string reason = register_master();
	if(reason.length() != 0)
		std::cout << "Master Server registration failed: " << reason << std::endl;
	else
		std::cout << "Master Server registration successful." << std::endl;

	Match match;
	if(!match){
		std::cout<<"error: couldn't bind to port "<<TCP_PORT<<" and/or port "<<UDP_PORT<<std::endl;
		std::cout<<"make sure no other server is using ports "<<TCP_PORT<<" (TCP) and "<<UDP_PORT<<" (UDP)"<<std::endl;
		return 1;
	}

	std::cout<<"[ready on tcp:"<<TCP_PORT<<" udp:"<<UDP_PORT<<"]"<<std::endl;

	while(run.load()){
		send_heartbeat(match);

		match.accept_new_clients();

		match.step();

		match.wait_next_step();
	}

	puts("\ngoodbye");
	return 0;
}

static void send_string(net::tcp &tcp, const std::string &str){
	const std::uint32_t count = str.length();

	tcp.send_block(&count, sizeof(count));
	tcp.send_block(str.c_str(), count);
}

static std::string get_string(net::tcp &tcp){
	std::uint32_t count;

	tcp.recv_block(&count, sizeof(count));
	std::vector<char> data(count + 1);

	tcp.recv_block(&data[0], count);
	data[count] = 0;

	return {&data[0]};
}

static std::string get_config_path(){
#ifdef _WIN32
	const std::string path = "%userprofile%\\fishtank-server.config";
	char expanded[MAX_PATH + 1] = "(INVALID PATH)";
	ExpandEnvironmentStrings(path.c_str(), expanded, MAX_PATH);
	return expanded;
#else
	const std::string path = "~/.fishtank-server-config";
	wordexp_t p;
	wordexp(path.c_str(), &p, 0);
	std::string expanded = p.we_wordv[0];
	wordfree(&p);
	return expanded;
#endif // _WIN32
}

static ServerConfig generate_server_config(){
	std::cout << "[First Time Setup]\nServer Name: " << std::flush;
	std::string name;
	std::getline(std::cin, name);
	std::cout << "Location (e.g. Northern California): " << std::flush;
	std::string location;
	std::getline(std::cin, location);

	const std::string path = get_config_path();
	std::ofstream out(path);
	if(!out)
		return {name, location};

	out << name << std::endl;
	out << location << std::endl;

	std::cout << "written to \"" << path << "\"" << std::endl;

	return {name, location};
}

static ServerConfig get_server_config(){
	const std::string path = get_config_path();
	std::ifstream in(path);
	if(!in)
		return generate_server_config();

	std::string ip, name, location;

	std::getline(in, name);
	std::getline(in, location);

	std::cout << "Name: " << name << std::endl;
	std::cout << "Location: " << location << std::endl;

	return {name, location};
}

// attempt to register this server on the fishtank master server
std::string register_master(){
	ServerConfig sc = get_server_config();
	net::tcp tcp(MASTER, MASTER_PORT);

	if(!tcp.connect(5))
		return "Could not connect to " MASTER;

	// tell the master server that this is a registration attempt
	const std::uint8_t type = 1;
	tcp.send_block(&type, sizeof(type));

	send_string(tcp, sc.name);
	send_string(tcp, sc.location);

	// master server connectback test
	{
		net::tcp_server listener(TCP_PORT);
		if(!listener)
			return "Could not bind to port " + std::to_string(TCP_PORT);

		const int start = time(NULL);
		int sock = -1;
		while(sock == -1 && time(NULL) - start < 4){
			sock = listener.accept();
		}
		if(sock == -1)
			return "Could not receive connectback from server.\nCheck your network "
			"configuration and make sure the proper ports are forwarded through your router and firewall.";
		net::tcp tcp2(sock);
		if(!tcp2)
			return "Could not create listener socket";

		uint8_t test = 0;
		tcp2.recv_block(&test, sizeof(test));
		if(test != 1)
			return "connectback test failed, received " + std::to_string(test) + " instead of 1";
	}

	// get result
	std::uint8_t success;
	tcp.recv_block(&success, sizeof(success));

	std::string reason;
	if(success == 0){
		// get reason
		reason = get_string(tcp);
	}

	if(tcp.error())
		return "A network error ocurred.";

	master = std::move(tcp);
	registered = true;

	return reason;
}

// send a heartbeat to the master server to let it know i am still connected
#define MASTER_HEARTBEAT_FREQUENCY 16 // seconds
void send_heartbeat(const Match &match){
	if(!registered)
		return;

	static int last = 0;

	const int current = time(NULL);
	if(current - last > MASTER_HEARTBEAT_FREQUENCY){
		last = current;

		// send heartbeat
		uint8_t count = match.client_list.size();
		master.send_block(&count, sizeof(count));

		if(master.error()){
			registered = false;
			std::cout << ANSI_RED << "Error:" << ANSI_RESET << " lost connection to master server."<<std::endl;
		}
	}
}

#ifndef _WIN32
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
