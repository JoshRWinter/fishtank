#ifndef MENU_H
#define MENU_H

struct Renderer;
struct State;
struct Base;

struct ServerConfig{
	ServerConfig(const std::string &i, const std::string &n, const std::string &loc, int c)
	:ip(i)
	,name(n)
	,location(loc)
	,count(c){}

	std::string ip;
	std::string name;
	std::string location;
	int count;
};

struct Menu{
	virtual void render(const Renderer&)const=0;
};

struct MenuMain:Menu{
	bool exec(State&);
	virtual void render(const Renderer&)const;

	Button button_play;
	Button button_quit;
	Button button_config;
	UIBase background;
};

struct MenuConfig:Menu{
	bool exec(State&);
	virtual void render(const Renderer&)const;

	UIBase background;
	Button button_music;
	Button button_sounds;
	Button button_vibrate;
	Button button_about;
	Button button_back;
	Config *config;
};

struct MenuInput:Menu{
	bool exec(State&,const char*,std::string*);
	virtual void render(const Renderer&)const;

	const char *header;
	ButtonSmall alpha[28];
	ButtonSmall numeric[10];
	ButtonSmall backspace,clear,space;
	UIBase background;
	std::string *text;
	ButtonSmall enter,cancel;
};

struct MenuPlay:Menu{
	bool exec(State&);
	virtual void render(const Renderer&)const;

	int *colorid;
	std::string *name;
	UIBase background;
	Button button_name;
	Button button_connect;
	Button button_back;
	UIBase blob[6]; // color blobs
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

	UIBase background;
	std::string address;
	std::string connected_address;
	Button button_cancel;
	Button button_ready;
	int connection_state;
};

struct MenuBrowserConnect{
	bool exec(State&);
	virtual void render(const Renderer&)const;

	UIBase background;
	std::vector<ServerConfig> servers;
	Button cancel;
};

// server browser
struct MenuBrowser{
	bool exec(State&, const std::vector<ServerConfig>&);
	virtual void render(const Renderer&)const;

	float anchor, trueanchor;
	bool touching;
	float scrolly;
	bool tapzone;

	UIBase background;
	const std::vector<ServerConfig> *list;
	Button cancel, direct;
};

struct MenuPause{
	bool exec(State&);
	virtual void render(const Renderer&)const;

	UIBase background,black_background;
	Button button_quit;
	Button button_settings;
	Button button_back;
	std::vector<stat> *scoreboard;
	int id; // my client id
};

struct MenuChat{
	bool exec(State&);
	void render(const Renderer&,const std::vector<ChatMessage>&)const;

	float scrolltop;
	bool drag; // currently draggin finger (scrolling)
	float offset;
	UIBase background;
	Button button_say;
	Button button_back;
};

struct MenuMessage:Menu{
	bool exec(State&,const char*,const char* =NULL);
	virtual void render(const Renderer&)const;

	const char *msg;
	const char *header;
	Button button_ok;
	UIBase background;
};

#endif // MENU_H
