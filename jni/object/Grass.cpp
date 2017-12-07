#include "../fishtank.h"

Grass::Grass(const Platform &platform, int type, float xoff){
	texture=AID_GRASS1+type;
	switch(type){
	case 0:
		w=GRASS1_WIDTH;
		h=GRASS1_HEIGHT;
		break;
	default:
		logcat("INVALID GRASS TYPE: %d",type);
	}
	x=platform.x+xoff;
	y=platform.y-h+0.1f;
	rot=0.0f;
}

void Grass::process(std::vector<Grass> &grass_list){
	for(Grass &grass:grass_list){
	}
}

void Grass::render(const Renderer &renderer,const std::vector<Grass> &grass_list){
	for(const Grass &grass:grass_list){
		renderer.draw(grass, &renderer.atlas);
	}
}
