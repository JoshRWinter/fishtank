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
#define PLAYER_Y_SPEED 0.03f
struct Player:Base{
	Player();
	static void process(Match&);

	int health;
	float xv,yv;
};

#endif // OBJECT_H
