#ifndef OBJECT_H
#define OBJECT_H

#define COLLIDE_LEFT 1
#define COLLIDE_RIGHT 2
#define COLLIDE_BOTTOM 3
#define COLLIDE_TOP 4
struct Base{
	bool collide(const Base&,float=0.0f)const;
	int correct(const Base&);

	float x,y,w,h;
};

struct Beacon:Base{
	Beacon();
	void reset();
	void fire(const Client&);

	float rot;
	float rotv;
	float xv,yv;
	float ttl;
};
#define PLAYER_X_SPEED 0.03f
#define PLAYER_Y_SPEED 0.1f
#define PLAYER_X_FAST_SPEED 0.035f
#define PLAYER_ANGLE_INCREMENT 0.02f
#define PLAYER_TIMER_FIRE 7
struct Mine;
struct Player:Base{
	Player(const area_bounds&,const std::vector<Mine>&,int);
	bool near_mine(const std::vector<Mine>&)const;
	void reset(const area_bounds&,const std::vector<Mine>&);
	static void process(Match&);

	Beacon beacon;
	bool avail_airstrike; // player is allowed to call an airstrike
	int health;
	int timer_fire;
	float angle; // angle of the turret
	float xv,yv;

};

#define SHELL_DMG 61,80
struct Shell:Base{
	Shell(const Match&,Client&,float);
	static void process(Match&);

	float xv,yv;
	const float power;
	Client &owner;
};

struct Platform:Base{
	Platform(bool,float,float);
	static void create_all(Match&);
	static void process(const std::vector<Platform>&);
	static void update(const std::vector<Platform>&);

	static uint32_t platform_status[2];
	bool horiz; // flat or vertical
	int health;
	int killed_by_id; // destroyed by playerid
	unsigned seed; // serves to synchronize random interactions between clients and server
};

#define ARTILLERY_SPLASH_RADIUS 5.2f
#define ARTILLERY_INITIAL_TIMER 110
#define ARTILLERY_TIMER 32,55
#define ARTILLERY_COUNT 5
struct Artillery:Base{
	Artillery(const Beacon &beacon,int);
	void explode(std::vector<Client*>&);
	static int dmg(float);

	int client_id; // client id who called in the artillery
	bool processed; // process by Match::send_data()
	float xv;
};
struct Airstrike{
	Airstrike(const Client&);
	~Airstrike();
	static void process(Match&);

	const Client &client;
	int timer; // timer till next artillery
	int count; // artillery left
	std::vector<Artillery*> arty_list;
};

#define MINE_BLAST_RADIUS 4.5f
struct Mine:Base{
	Mine(const std::vector<Platform>&,int);
	void explode(std::vector<Platform>&,std::vector<Client*>&);
	static void create_all(Match&);
	static void process(Match &match);
	static void update(const std::vector<Mine>&);
	static int dmg(float);
	static uint32_t mine_status;

	int platform_index;
	int disturbed_by; // player id who last disturbed the mine
	bool armed;
	float yv;
};

#endif // OBJECT_H
