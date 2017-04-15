#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "network.h"

socket_tcp_server::socket_tcp_server(){
	scan=-1;
}

// creates and binds a socket
// true on success
// false on failure (most comman cause for failure: someone else is already bound to <port>)
bool socket_tcp_server::bind(unsigned short port){
	sockaddr_in6 addr;
	memset(&addr,0,sizeof(sockaddr_in6));
	addr.sin6_family=AF_INET6;
	addr.sin6_port=htons(port);
	addr.sin6_addr=in6addr_any;

	// create the socket for scanning
	scan=socket(AF_INET6,SOCK_STREAM,IPPROTO_TCP);
	if(scan==-1)
		return false;
	// non blocking
	fcntl(scan,F_SETFL,fcntl(scan,F_GETFL,0)|O_NONBLOCK);
	// make this socket reusable
	int reuse=1;
	setsockopt(scan,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));
	// bind this socket to <port>
	if(-1==::bind(scan,(sockaddr*)&addr,sizeof(sockaddr_in6)))
		return false;

	listen(scan,SOMAXCONN);
	return true;
}

// BLOCKS and returns the connecting socket
// returns -1 on failure
int socket_tcp_server::accept(std::string &name){
	sockaddr_in6 connector_addr;
	socklen_t addr_len=sizeof(sockaddr_in6);
	int sock=::accept(scan,(sockaddr*)&connector_addr,&addr_len);
	if(sock==-1)
		return sock;
	char info[51];
	// get the ip of the connecter
	getnameinfo((sockaddr*)&connector_addr,sizeof(sockaddr_in6),info,51,NULL,0,NI_NUMERICHOST);
	name=info;
	return sock;
}

void socket_tcp_server::close(){
	if(scan != -1){
		::close(scan);
		scan=-1;
	}
}

socket_tcp_server::~socket_tcp_server(){
	this->close();
}

// SOCKET TCP CLIENT
// initialize with already opened socket
socket_tcp::socket_tcp(int socket){
	sock=socket;
	ai=NULL;
}

socket_tcp::socket_tcp(){
	sock=-1;
	ai=NULL;
}

// attempt to connect to <address> on <port>, fills <name> with canonical name of <address>, returns true on success
bool socket_tcp::setup(const std::string &address,unsigned short port){
	addrinfo hints;

	memset(&hints,0,sizeof(addrinfo));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_protocol=IPPROTO_TCP;

	// convert port to string
	char port_string[20];
	sprintf(port_string,"%hu",port);

	// resolve hostname
	if(0!=getaddrinfo(address.c_str(),port_string,&hints,&ai)){
		return false;
	}

	char info[51];
	if(0!=getnameinfo(ai->ai_addr,ai->ai_addrlen,info,51,NULL,0,NI_NUMERICHOST)){
		return false;
	}
	name=info;

	// create the socket
	sock=socket(ai->ai_family,ai->ai_socktype,ai->ai_protocol);
	if(sock==-1){
		return false;
	}
	// non blocking
	fcntl(sock,F_SETFL,fcntl(sock,F_GETFL,0)|O_NONBLOCK);

	return true;
}

bool socket_tcp::connect(){
	if(sock==-1)
		return false;

	bool result=::connect(sock,ai->ai_addr,ai->ai_addrlen)==0;

	// back to blocking
	if(result)
		fcntl(sock,F_SETFL,fcntl(sock,F_GETFL,0)&~O_NONBLOCK);

	return result;
}

void socket_tcp::send(const void *buffer,unsigned size){
	if(sock==-1)
		return;

	int tr=0; // bytes transferred
	while(tr!=size){
		int result=::send(sock,((char*)buffer)+tr,size-tr,0);
		if(result<1){
			this->close();
			return;
		}
		else
			tr+=result;
	}
}

void socket_tcp::recv(void *buffer,unsigned size){
	if(sock==-1)
		return;

	int tr=0; // bytes transferred
	while(tr!=size){
		int result=::recv(sock,((char*)buffer)+tr,size-tr,0);
		if(result<1){
			this->close();
			return;
		}
		else
			tr+=result;
	}
}

int socket_tcp::peek(){
	if(sock==-1)
		return 0;

	int available=0;
	ioctl(sock,FIONREAD,&available);
	return available;
}

bool socket_tcp::error(){
	return sock==-1;
}

void socket_tcp::get_name(std::string &s){
	s=name;
}

void socket_tcp::close(){
	if(sock!=-1){
		::close(sock);
		sock=-1;
	}
	if(ai!=NULL){
		freeaddrinfo(ai);
		ai=NULL;
	}
}

socket_tcp::~socket_tcp(){
	this->close();
}

// UDP
socket_udp_server::socket_udp_server(){
	sock=-1;
}

socket_udp_server::~socket_udp_server(){
	this->close();
}

bool socket_udp_server::bind(unsigned short port){
	addrinfo hints,*ai;

	memset(&hints,0,sizeof(addrinfo));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_flags=AI_PASSIVE;

	// convert port to string
	char port_string[20];
	sprintf(port_string,"%hu",port);

	// resolve hostname
	if(0!=getaddrinfo(NULL,port_string,&hints,&ai)){
		this->close();
		return false;
	}

	// create the socket
	sock=socket(ai->ai_family,ai->ai_socktype,ai->ai_protocol);
	if(sock==-1){
		this->close();
		return false;
	}

	// bind to port
	int reuse=1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));
	if(::bind(sock,ai->ai_addr,ai->ai_addrlen)){
		this->close();
		return false;
	}

	freeaddrinfo(ai);

	return true;
}

void socket_udp_server::close(){
	if(sock!=-1){
		::close(sock);
		sock=-1;
	}
}

void socket_udp_server::send(const void *buffer,unsigned len,const udp_id &id){
	if(sock==-1)
		return;
	if(!id.initialized){
		this->close();
		return;
	}

	// no such thing as partial sends for sendto with udp
	int result=sendto(sock,buffer,len,0,(sockaddr*)&id.storage,id.len);
	if(result!=len){
		this->close();
		return;
	}
}

void socket_udp_server::recv(void *buffer,unsigned len,udp_id &id){
	if(sock==-1)
		return;

	// no partial receives
	int result=recvfrom(sock,buffer,len,0,(sockaddr*)&id.storage,&id.len);
	if(result!=len){
		this->close();
		return;
	}
	id.initialized=true;
}

int socket_udp_server::peek(){
	if(sock==-1)
		return 0;

	int available=0;
	ioctl(sock,FIONREAD,&available);

	return available;
}

bool socket_udp_server::error(){
	return sock==-1;
}

// udp client
socket_udp::socket_udp(){
	sock=-1;
	ai=NULL;
}

socket_udp::~socket_udp(){
	this->close();
}

// remember it is auto bound no need to explicitly bind
bool socket_udp::setup(const std::string &address,unsigned short port){
	addrinfo hints;

	memset(&hints,0,sizeof(addrinfo));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=0;

	// convert port to string
	char port_string[20];
	sprintf(port_string,"%hu",port);

	// resolve hostname
	if(0!=getaddrinfo(address.c_str(),port_string,&hints,&ai)){
		return false;
	}

	// create the socket
	sock=socket(ai->ai_family,ai->ai_socktype,ai->ai_protocol);
	if(sock==-1){
		return false;
	}

	return true;
}

void socket_udp::send(const void *buffer,unsigned len){
	if(sock==-1)
		return;

	// no such thing as a partial send for udp with sendto
	ssize_t result=sendto(sock,buffer,len,0,(sockaddr*)ai->ai_addr,ai->ai_addrlen);
	if(result!=len){
		this->close();
		return;
	}
}

void socket_udp::recv(void *buffer,unsigned len){
	if(sock==-1)
		return;

	// ignored
	sockaddr_storage src_addr;
	socklen_t src_len=sizeof(sockaddr_storage);

	// no such thing as a partial send for udp with sendto
	ssize_t result=recvfrom(sock,buffer,len,0,(sockaddr*)&src_addr,&src_len);
	if(result!=len){
		this->close();
		return;
	}
}

int socket_udp::peek(){
	if(sock==-1)
		return 0;

	int avail=0;
	ioctl(sock,FIONREAD,&avail);

	return avail;
}

bool socket_udp::error(){
	return sock==-1;
}

void socket_udp::close(){
	if(sock!=-1){
		::close(sock);
		sock=-1;
	}
	if(ai!=NULL){
		freeaddrinfo(ai);
		ai=NULL;
	}
}
