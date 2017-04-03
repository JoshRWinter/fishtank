#include <string>
#include <netinet/in.h>
#include <netdb.h>
class socket_tcp_server{
public:
	~socket_tcp_server();

	bool bind(unsigned short);
	int accept(std::string&);
	void close();

private:
	int scan; // the socket for scanning
};

class socket_tcp{
public:
	socket_tcp();
	socket_tcp(int);
	~socket_tcp();

	bool setup(const std::string &address,std::string&,unsigned short);
	bool connect();
	void send(const void*,unsigned);
	void recv(void*,unsigned);
	void close();
	bool error();

private:
	int sock;
	addrinfo addr_info;
};
