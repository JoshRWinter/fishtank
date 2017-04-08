class Match{
public:
	Match();
	~Match();
	void initialize(const std::string&,socket_tcp&);
	void quit();
	void send_data(State&);
	void recv_data(State&);
	bool connected();

private:
	socket_tcp tcp;
};
