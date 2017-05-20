#ifndef MATCH_H
#define MATCH_H

#define DEAD_TIMER 400 // after the player dies and is allowed to spectate

class Match{
public:
	Match();
	~Match();
	void initialize(State&);
	void quit();
	void send_data(const State&);
	void recv_data(State&);
	void get_level_config(State&);
	void get_scoreboard(std::vector<stat>&);
	void request_scoreboard();
	bool connected();
	void send_chat(const std::string&);
	void cycle_spectate(const std::vector<Player>&);
	socket_tcp &get_tcp();

	int my_index; // player's index into State::player_list
	int round_id;
	int dead_timer; // after the player dies and is allowed to spectate
	int spectate_index; // index of player whom is being spectated

private:
	int32_t udp_secret;
	socket_tcp tcp;
	socket_udp udp;
	int id; // client id
};

#endif // MATCH_H
