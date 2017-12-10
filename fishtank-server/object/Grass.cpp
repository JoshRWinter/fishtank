#include "../fishtank-server.h"

static int decide_grass_type(){
	int p = randomint(0, 999);

	if(p < 350)
		return 4;
	if(p < 520)
		return 3;
	if(p < 600)
		return 2;
	if(p  < 810)
		return 0;

	return 1;
}

Grass::Grass(int plat_index){
	type = decide_grass_type();
	xoffset = randomint(0, (PLATFORM_WIDTH-grass_width(type))*100.0f)/100.0f;
	platform_index = plat_index;
}

void Grass::generate(std::vector<Grass> &list, const std::vector<Platform> &platforms){
	// clear from last round
	list.clear();

	for(unsigned i = 0; i < platforms.size(); ++i){
		if(onein(2) || !platforms[i].horiz || platforms[i].health == 0)
			continue;

		// decide number of grasses to generate on this platform
		int count = 1;
		if(onein(5))
			count = 2;

		for(int j = 0; j < count; ++j)
			list.push_back(i);
	}
}
