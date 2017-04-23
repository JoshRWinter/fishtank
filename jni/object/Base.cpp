#include "../fishtank.h"

// collision detection
bool Base::collide(const Base &b,float tolerance)const{
	return x+w>b.x+tolerance&&x<(b.x+b.w)-tolerance&&y+h>b.y+tolerance&&y<(b.y+b.h)-tolerance;
}

// collision correction
int Base::correct(const Base &b){
	if(!collide(b))
		return 0;

	float rdiff=fabs(x-(b.x+b.w)); // diff between right side of b and left side of this
	float ldiff=fabs((x+w)-b.x);
	float tdiff=fabs((y+h)-b.y);
	float bdiff=fabs(y-(b.y+b.h));

	// find which is the smallest
	float smallest=rdiff;
	int side=COLLIDE_RIGHT;
	if(ldiff<smallest){
		smallest=ldiff;
		side=COLLIDE_LEFT;
	}
	if(tdiff<smallest){
		smallest=tdiff;
		side=COLLIDE_TOP;
	}
	if(bdiff<smallest){
		smallest=bdiff;
		side=COLLIDE_BOTTOM;
	}

	switch(side){
	case COLLIDE_RIGHT:
		x=b.x+b.w;
		return side;
		break;
	case COLLIDE_LEFT:
		x=b.x-w;
		return side;
		break;
	case COLLIDE_TOP:
		y=b.y-h;
		return side;
		break;
	case COLLIDE_BOTTOM:
		y=b.y+b.h;
		return side;
		break;
	}
	return 0;
}

bool Base::pointing(const crosshair &ch,float tolerance)const{
	return ch.x>x+tolerance&&ch.x<(x+w)-tolerance&&ch.y>y+tolerance&&ch.y<(y+h)-tolerance;
}

void Base::init_background(Base &b,const Renderer &renderer){
	b.x=renderer.view.left;
	b.y=renderer.view.top;
	b.w=renderer.view.right*2.0f;
	b.h=renderer.view.bottom*2.0f;
	b.rot=0.0f;
	b.frame=1;
	b.count=0;
}
