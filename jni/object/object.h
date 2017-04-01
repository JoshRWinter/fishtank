#define COLLIDE_LEFT 1
#define COLLIDE_RIGHT 2
#define COLLIDE_BOTTOM 3
#define COLLIDE_TOP 4

struct Base{
	bool collide(const Base&)const;
	int correct(const Base&)const;

	float x,y,w,h,rot;
	int frame,count;
};
