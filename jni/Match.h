class Match{
public:
	Match();
	~Match();
	void initialize(const std::string&);
	void quit();
	void send_data(const State&);
	void recv_data(State&);
	bool connected();
	socket_tcp &get_tcp();

	int my_index; // player's index into State::player_list

private:
	int32_t udp_secret;
	socket_tcp tcp;
	socket_udp udp;
	int id; // client id
};
