#ifndef MENU_H
#define MENU_H

struct Renderer;
struct State;
struct Base;

struct Menu{
	virtual void render(const Renderer&)const=0;
};

struct MenuMain:Menu{
	bool exec(State&);
	virtual void render(const Renderer&)const;

	Button button_play;
	Button button_quit;
	Base background;
};

struct MenuInput:Menu{
	bool exec(State&,const char*,std::string*);
	virtual void render(const Renderer&)const;

	const char *header;
	ButtonSmall alpha[28];
	ButtonSmall numeric[10];
	ButtonSmall backspace,space;
	Base background;
	std::string *text;
	ButtonSmall enter,cancel;
};

struct MenuPlay:Menu{
	bool exec(State&);
	virtual void render(const Renderer&)const;

	std::string *name;
	Base background;
	Button button_name;
	Button button_connect;
	Button button_back;
};

#define CONN_STATE_TRYING 1
#define CONN_STATE_DEAD 2
#define CONN_STATE_TIMEOUT 3
#define CONN_STATE_READY 4
#define CONN_STATE_KICKED 5
#define CONNECTION_TIMEOUT 15
struct MenuConnect{
	bool exec(State&,const std::string&);
	virtual void render(const Renderer&)const;

	Base background;
	std::string address;
	std::string connected_address;
	Button button_cancel;
	Button button_ready;
	int connection_state;
};

struct MenuChat{
	bool exec(State&);
	void render(const Renderer&,const std::vector<ChatMessage>&)const;

	float scrolltop;
	bool drag; // currently draggin finger (scrolling)
	float offset;
	Base background;
	Button button_say;
	Button button_back;
};

#endif // MENU_H
