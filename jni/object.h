#ifndef OBJECT_H
#define OBJECT_H

#include <string>

#define COLLIDE_LEFT 1
#define COLLIDE_RIGHT 2
#define COLLIDE_BOTTOM 3
#define COLLIDE_TOP 4

struct Base{
	bool collide(const Base&)const;
	bool pointing(const crosshair&)const;
	int correct(const Base&);

	float x,y,w,h,rot;
	int frame,count;
};

#define BUTTON_WIDTH 3.0f
#define BUTTON_HEIGHT BUTTON_WIDTH
struct Button:Base{
	void init(float,float,const char*);
	bool process(const crosshair*);
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

#endif // OBJECT_H
