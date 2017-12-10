#include "../fishtank.h"

Grass::Grass(const Platform &platform, int type, float xoff, bool flipped){
	plat = &platform;
	texture=AID_GRASS1+type;
	w = grass_width(type);
	h = grass_height(type);
	x=platform.x+xoff;
	y=platform.y-h+0.1f;
	rot=0.0f;

	// maybe x flip
	if(flipped){
		x += w;
		w = -w;
	}
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

void Grass::destroy(const std::vector<Platform> &platforms, std::vector<Grass> &grass, int index){
	for(auto it = grass.begin(); it != grass.end();){
		if(it->plat == &platforms[index]){
			it = grass.erase(it);
			continue;
		}

		++it;
	}
}
