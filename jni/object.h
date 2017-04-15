#ifndef OBJECT_H
#define OBJECT_H

struct Base{
	bool collide(const Base&)const;
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
	bool cue_fire;
};

#endif // OBJECT_H
