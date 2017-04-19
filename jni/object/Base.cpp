#include "../fishtank.h"

bool Base::collide(const Base &b,float tolerance)const{
	return x+w>b.x+tolerance&&x<(b.x+b.w)-tolerance&&y+h>b.y+tolerance&&y<(b.y+b.h)-tolerance;
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
