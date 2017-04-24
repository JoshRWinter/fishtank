#ifndef OBJECT_H
#define OBJECT_H

#define FIRE_BUTTON_SIZE 1.8f

#define COLLIDE_LEFT 1
#define COLLIDE_RIGHT 2
#define COLLIDE_BOTTOM 3
#define COLLIDE_TOP 4
struct Base{
	bool collide(const Base&,float=0.0f)const;
	int correct(const Base&);
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

#define PLATFORM_VIS_WIDTH PLATFORM_WIDTH
#define PLATFORM_VIS_HEIGHT 0.85f
struct Platform:Base{
	Platform(bool,bool,float,float);
	static void render(const Renderer&,const std::vector<Platform>&);

	Base visual;
	bool active;
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
#define PARTICLE_PLATFORM_SIZE 0.2f
struct ParticlePlatform:Base{
	ParticlePlatform(const Shell&);
	ParticlePlatform(float,float);
	static void spawn(State&,const Shell&,int);
	static void spawn_destroy_platform(State&,const Platform&);
	static void process(State&);
	static void render(const Renderer&,const std::vector<ParticlePlatform*>&);

	float xv,yv;
	float ttl;
};

#define PARTICLE_PLAYER_SIZE 0.2f
#define PARTICLE_PLAYER_LARGE_SIZE 0.425f
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
};

#endif // OBJECT_H
