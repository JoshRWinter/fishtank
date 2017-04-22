#ifndef OBJECT_H
#define OBJECT_H

#define FIRE_BUTTON_SIZE 1.8f

struct Base{
	bool collide(const Base&,float=0.0f)const;
	bool pointing(const crosshair&,float=0.0f)const;
	static void init_background(Base&,const Renderer&);

	float x,y,w,h,rot;
	int frame,count;
};

#define BUTTON_WIDTH 3.0f
#define BUTTON_HEIGHT BUTTON_WIDTH
struct Button:Base{
	void init(float,float,const char*);
	bool process(const crosshair*,float=0.0f);
	void render(const Renderer&)const;
	void render_text(const Renderer&)const;

	bool active;
	const char *label;
};

#define BUTTON_SMALL_SIZE 1.05f
struct ButtonSmall:Button{
	void init(float,float,const char*);
	void render_text(const Renderer&)const;
};

struct ButtonBasic:Button{
	void init(float,float,float,const char*);
	void render_text(const Renderer&)const;
};

struct Player:Base{
	Player();
	static void process(State&);
	static void render(const Renderer&,const std::vector<Player>&);

	float xv,yv;
	Base turret;
	int colorid; // zero if empty player
	int health;
	float cue_fire;
};

#define SHELL_VIS_WIDTH 0.0f
#define SHELL_VIS_HEIGHT 0.2f
struct Shell:Base{
	Shell(const State&,const Player&);
	static void process(State&);
	static void render(const Renderer&,const std::vector<Shell*>&);

	float xv,yv;
	const Player &owner;
	Base visual; // the shell that is rendered
};

struct Platform:Base{
	Platform(bool,bool,float,float);
	static void render(const Renderer&,const std::vector<Platform>&);

	Base visual;
	bool active;
};

#endif // OBJECT_H
