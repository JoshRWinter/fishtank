#ifndef MATCH_H
#define MATCH_H

class Match{
public:
	Match();
	~Match();
	void initialize(State&);
	void quit();
	void send_data(const State&);
	void recv_data(State&);
	void get_level_config(State&);
	bool connected();
	void send_chat(const std::string&);
	socket_tcp &get_tcp();

	int my_index; // player's index into State::player_list

private:
	int32_t udp_secret;
	socket_tcp tcp;
	socket_udp udp;
	int id; // client id
};

#endif // MATCH_H
