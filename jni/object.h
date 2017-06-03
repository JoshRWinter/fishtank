#ifndef OBJECT_H
#define OBJECT_H

#define FIRE_BUTTON_SIZE 1.8f
#define TEXT_COLOR 0.1f,0.1f,0.1f

#define SKULL_WIDTH 0.4f
#define SKULL_HEIGHT 0.475f

#define COLLIDE_LEFT 1
#define COLLIDE_RIGHT 2
#define COLLIDE_BOTTOM 3
#define COLLIDE_TOP 4
struct Base{
	bool collide(const Base&,float=0.0f)const;
	int correct(const Base&);
	bool pointing(const crosshair&,float=0.0f)const;
	void init_background(const Renderer&);
	float dist(const Base&);

	float x,y,w,h,rot;
	int frame,count;
};

#define BUTTON_WIDTH 4.0f
#define BUTTON_HEIGHT 1.0f
#define BUTTON_TEXT_COLOR 0.175f,0.175f,0.175f
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
	void render(const Renderer&)const;
	void render_text(const Renderer&)const;
};

#define BEACON_FRAME_TIMER 18
struct Beacon:Base{
	Beacon();

	float xv,yv;
	float ttl;
	float timer_frame;
};
struct Player:Base{
	Player();
	static void process(State&);
	static void render(const Renderer&,const std::vector<Player>&);

	Beacon beacon;
	float xv,yv;
	Base turret;
	int colorid; // zero if empty player
	int health;
	float cue_fire;
	struct{
		int engine;
		int bubbles;
	}audio;
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

#define PLATFORM_VIS_WIDTH PLATFORM_WIDTH
#define PLATFORM_VIS_HEIGHT 0.85f
struct Platform:Base{
	Platform(bool,bool,float,float,unsigned);
	static void process(State&);
	static void render(const Renderer&,const std::vector<Platform>&);

	Base visual;
	bool active;
	int health;
	float timer_audio;
	unsigned seed; // serves to sync random interactions between client and server
};

#define ARTILLERY_WIDTH 0.35f
#define ARTILLERY_HEIGHT 0.1375f
struct Artillery:Base{
	Artillery(float,float);
	static void process(State&);
	static void render(const Renderer&,const std::vector<Artillery*>&);

	Base visual; // the visible artillery. inherited Base is for collision purposes only
	float xv;
};

struct Mine:Base{
	Mine(const std::vector<Platform>&,int,bool);
	static void process(State&);
	static void render(const Renderer&,const std::vector<Mine>&);

	int platform_index;
	Base chain;
	bool armed;
	float yv;
};

#define PARTICLE_SHELL_HEIGHT 0.1333f
#define PARTICLE_SHELL_TTL 6,11
struct ParticleShell:Base{
	ParticleShell(const Shell&);
	static void spawn(State&,const Shell&,int);
	static void process(State&);
	static void render(const Renderer&,const std::vector<ParticleShell*>&);

	float xv,yv;
	float ttl;
	int colorid; // from the player who fired the shell generating this particle
};

#define PARTICLE_PLATFORM_SPEED 0.1f
#define PARTICLE_PLATFORM_TTL 145.0f
#define PARTICLE_PLATFORM_SIZE 14,26
struct ParticlePlatform:Base{
	ParticlePlatform(const Shell&);
	ParticlePlatform(float,float);
	ParticlePlatform(const Artillery&);
	static void spawn(State&,const Shell&,int);
	static void spawn_destroy_platform(State&,const Platform&);
	static void spawn(State&,const Artillery&);
	static void process(State&);
	static void render(const Renderer&,const std::vector<ParticlePlatform*>&);

	float xv,yv;
	float ttl;
};

#define PARTICLE_PLAYER_SIZE (randomint(15,28)/100.0f)
#define PARTICLE_PLAYER_LARGE_SIZE (randomint(39,61)/100.0f)
struct ParticlePlayer:Base{
	ParticlePlayer(float,float,bool,int);
	static void spawn(State&,const Player&);
	static void spawn(State&,const Shell&,int);
	static void process(State&);
	static void render(const Renderer&,const std::vector<ParticlePlayer*>&);

	float xv,yv,rotv;
	float ttl;
	bool active;
	int colorid;
	float timer_lifetime;
	float large;
};

#define PARTICLE_BUBBLE_SIZE 0.175f
struct ParticleBubble:Base{
	ParticleBubble(const State&,const Player&);
	ParticleBubble(const Artillery&);
	ParticleBubble(const Mine&);
	static void spawn(State&,const Mine&);
	static void process(State&);
	static void render(const Renderer&,const std::vector<ParticleBubble*>&);

	float xv,yv;
	float ttl;
};

#define FISH_WIDTH 0.9f
#define FISH_HEIGHT 0.4f
struct DeadFish:Base{
	DeadFish(float,float,int);
	static void process(State&);
	static void render(const Renderer&,const std::vector<DeadFish*>&);

	int colorid;
	float xv,yv,rotv;
	float increase;
	bool stuck;
};

#endif // OBJECT_H
