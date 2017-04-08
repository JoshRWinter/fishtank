#include "../jni/fishnet.h"
#include "../jni/network.h"

struct Client{
	Client(int);
	~Client();
	void kick();

	socket_tcp tcp;
	std::string name;
};

void get_nano_time(long long*);
