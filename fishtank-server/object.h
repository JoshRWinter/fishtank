#ifndef OBJECT_H
#define OBJECT_H

#define COLLIDE_LEFT 1
#define COLLIDE_RIGHT 2
#define COLLIDE_BOTTOM 3
#define COLLIDE_TOP 4
struct Base{
	bool collide(const Base&,float=0.0f)const;
	int correct(const Base&);

	float x,y,w,h,rot;
};

#define PLAYER_X_SPEED 0.03f
#define PLAYER_Y_SPEED 0.1f
#define PLAYER_ANGLE_INCREMENT 0.02f
struct Player:Base{
	Player(const area_bounds&);
	static void process(Match&);
	void reset(const area_bounds&);

	int health;
	float angle; // angle of the turret
	float xv,yv;

};

#define SHELL_DMG 11,20
struct Shell:Base{
	Shell(const Match&,Client&);
	static void process(Match&);

	float xv,yv;
	Client &owner;
};

struct Platform:Base{
	Platform(bool,float,float);
	static void create_all(Match&);
	static void process(const std::vector<Platform>&);

	static uint32_t platform_status[2];
	bool horiz; // flat or vertical
	int health;
};

#endif // OBJECT_H
