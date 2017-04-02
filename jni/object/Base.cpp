#include "../fishtank.h"

bool Base::pointing(const crosshair &ch)const{
	return ch.x>x&&ch.x<x+w&&ch.y>y&&ch.y<y+h;
}
