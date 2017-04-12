class Match{
public:
	Match();
	~Match();
	void initialize(const std::string&,socket_tcp&);
	void quit();
	void send_data(const State&);
	void recv_data(State&);
	bool connected();

private:
	int32_t udp_secret;
	socket_tcp tcp;
	socket_udp udp;
};
