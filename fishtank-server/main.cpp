#include <atomic>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <fstream>
#include <iostream>

#ifndef _WIN32
#include <wordexp.h>
#include <sys/types.h>
#include <ifaddrs.h>
#endif // _WIN32

#include "fishtank-server.h"
#include "WebView.h"

struct CommandLine{
	std::string allowed_ip_address; // who is allowed to connect to web interface
};

static std::atomic<bool> run;

static std::string register_master(net::tcp&);
static bool send_heartbeat(net::udp&, const Match &match);
static bool recv_heartbeat(net::udp&);
static std::string get_my_ip_addr();
static CommandLine get_cmdline_args(int, char**);
static void restart(int, char**);

static int ctrl_c_count=0; // only accessed by signal_handler
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

int main(int argc, char **argv){
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

	// figure out cmd line args
	const CommandLine cmd = get_cmdline_args(argc, argv);

	const std::string &my_ip_addr = get_my_ip_addr();

	// register on the master server
	net::tcp master(MASTER, MASTER_PORT); // master tcp connection
	net::udp hbeater(MASTER, MASTER_PORT); // for heartbeats to master
	const std::string reason = register_master(master);
	master.close();
	bool registered = reason.length() == 0;
	if(reason.length() != 0)
		lprintf("Master Server registration failed: %s", reason.c_str());
	else
		lprintf("Master Server registration successful.");

	Match match;
	if(!match){
		lprintf("error: couldn't bind to port %d and/or port %d", TCP_PORT, UDP_PORT);
		lprintf("make sure no other server is using ports %d (TCP) or %d (UDP)", TCP_PORT, UDP_PORT);
		return 1;
	}

	WebView web(WEBVIEW_PORT, match, cmd.allowed_ip_address);
	if(!web){
		lprintf("error: couldn't bind to port %d", WEBVIEW_PORT);
		lprintf("make sure no other server is using that port.");
		return 1;
	}

	lprintf("[ready on tcp: %d udp: %d at \"%s\" web-interface: \"http://localhost:%d\"]", TCP_PORT, UDP_PORT, my_ip_addr.c_str(), WEBVIEW_PORT);

	int last_send_heartbeat = time(NULL);
	int last_recv_heartbeat = time(NULL);
	while(run.load()){
		if(registered){
			const int current = time(NULL);

			// send heartbeat
			if(current - last_send_heartbeat > MASTER_HEARTBEAT_FREQUENCY){
				registered = send_heartbeat(hbeater, match); // send a heartbeat to the master
				last_send_heartbeat = current;
			}

			// recv heartbeat
			if(recv_heartbeat(hbeater))
				last_recv_heartbeat = current;

			if(current - last_recv_heartbeat > MASTER_HEARTBEAT_FREQUENCY * 6){
				registered = false;
				lprintf(ANSI_RED "Error: " ANSI_RESET "lost connection to master");
			}
		}

		if(match.accept_new_clients() && registered)
			registered = send_heartbeat(hbeater, match);

		match.step();

		// handle the web view interface
		try{
			web.serve();
		}catch(const ShutdownException &e){
			lprintf("@@@@@@@@@@@@@@@ WebView shutdown");
			return 0;
		}
		catch(const RestartException &e){
			lprintf("@@@@@@@@@@@@@@@ WebView restart");
			web.~WebView();
			match.~Match();
			hbeater.~udp();
			master.~tcp();
			restart(argc, argv);
		}

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
	std::cout << "Location (e.g. Central Kentucky): " << std::flush;
	std::string location;
	std::getline(std::cin, location);

	const std::string path = get_config_path();
	std::ofstream out(path);
	if(!out)
		return {name, location};

	out << name << std::endl;
	out << location << std::endl;

	lprintf("written to \"%s\"", path.c_str());

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

	lprintf("Name: %s", name.c_str());
	lprintf("Location: %s", location.c_str());

	return {name, location};
}

// attempt to register this server on the fishtank master server
std::string register_master(net::tcp &master){
	ServerConfig sc = get_server_config();

	if(!master.connect(5))
		return "Could not connect to " MASTER;

	// tell the master server that this is a registration attempt
	const std::uint8_t type = 1;
	master.send_block(&type, sizeof(type));

	send_string(master, sc.name);
	send_string(master, sc.location);

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
	master.recv_block(&success, sizeof(success));

	std::string reason;
	if(success == 0){
		// get reason
		reason = get_string(master);
	}

	if(master.error())
		return "A network error ocurred.";

	return reason;
}

// send a heartbeat to the master server to let it know i am still connected
bool send_heartbeat(net::udp &udp, const Match &match){
	// send heartbeat
	std::uint8_t count = match.client_list.size();
	udp.send(&count, sizeof(count));

	if(udp.error()){
		lprintf(ANSI_RED "Error: " ANSI_RESET "failure when sending master heartbeat");
		return false;
	}

	return true;
}

// receives messages from master, returns true if it got one, false if there's no pending heartbeats
bool recv_heartbeat(net::udp &udp){
	std::uint8_t msg;
	if(udp.peek() > 0){
		udp.recv(&msg, sizeof(msg));

		return true;
	}

	return false;
}

std::string get_my_ip_addr(){
	std::string hostname;

#ifdef _WIN32
	HOSTENT *hostinfo;
	char host[200];

	if(gethostname(host, sizeof(host)) == 0){
		if((hostinfo = gethostbyname(host)) != NULL){
			hostname = inet_ntoa(*(in_addr*)*hostinfo->h_addr_list);
		}
	}

	if(hostname == "127.0.0.1" || hostname == "::1")
		hostname = "";
#else
	struct ifaddrs *head;
	getifaddrs(&head);

	ifaddrs *current = head;
	while(current != NULL){
		char host[200] = "";
		getnameinfo(current->ifa_addr, current->ifa_addr->sa_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6), host, sizeof(host), NULL, 0, NI_NUMERICHOST);

		if(strcmp(host, "127.0.0.1") && strcmp(host, "::1") && strcmp(host, "")){
			hostname = host;
			break;
		}

		current = current->ifa_next;
	}

	freeifaddrs(head);
#endif // _WIN32

	return hostname == "" ? "[undetermined]" : hostname;
}

static void usage(const char *program){
	lprintf("usage:\n%s [--allowed-ips=...]", program);
	lprintf("\n--allowed-ips=: Comma separated list of ip addresses that are allowed to access the web interface");
	lprintf("\tex. --allowed-ips=192.168.1.15,192.168.1.22");

	exit(0);
}

CommandLine get_cmdline_args(int argc, char **argv){
	CommandLine cmd;

	for(int i = 1; i < argc; ++i){
		const std::string option = argv[i];

		if(option == "--help" || option == "-help" || option == "-h")
			usage(argv[0]);
		else if(option.find("--allowed-ips=") != std::string::npos)
			cmd.allowed_ip_address = option.substr(14);
	}

	return cmd;
}

void restart(int argc, char **argv){
	execve(argv[0], argv, NULL);
	lprintf("could not exec, errno=%d", errno);
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
