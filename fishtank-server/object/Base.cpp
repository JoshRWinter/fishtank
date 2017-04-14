#include "../fishtank-server.h"

bool Base::collide(const Base &b,float tolerance)const{
	return x+w>b.x+tolerance&&x<(b.x+b.w)-tolerance&&y+h>b.y+tolerance&&y<(b.y+b.h)-tolerance;
}

int Base::correct(const Base &b){
	return 0;
}
