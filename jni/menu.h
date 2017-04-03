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

#define MSG_LIMIT 45
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

#endif // MENU_H
