#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <netinet/in.h>
#include <netdb.h>

// tcp
class socket_tcp_server{
public:
	socket_tcp_server();
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

	bool setup(const std::string &address,unsigned short);
	int disable();
	bool connect();
	void send(const void*,unsigned);
	void recv(void*,unsigned);
	int peek();
	void close();
	bool error();
	void get_name(std::string&);

private:
	int sock;
	std::string name;
	addrinfo *ai;
};

// udp
struct udp_id{
	udp_id():initialized(false),len(sizeof(sockaddr_storage)){}

	sockaddr_storage storage;
	socklen_t len;
	bool initialized;
};

class socket_udp_server{
public:
	socket_udp_server();
	~socket_udp_server();
	bool bind(unsigned short);
	void close();
	void send(const void*,unsigned,const udp_id&);
	void recv(void*,unsigned,udp_id&);
	int peek();
	bool error();

private:
	int sock;
};

class socket_udp{
public:
	socket_udp();
	~socket_udp();
	bool setup(const std::string&,unsigned short);
	void close();
	void send(const void*,unsigned);
	void recv(void*,unsigned);
	int peek();
	bool error();

private:
	int sock;
	addrinfo *ai;
};

#endif // NETWORK_H
