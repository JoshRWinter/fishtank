#include "../fishtank-server.h"

Grass::Grass(int plat_index){
	type = randomint(0, GRASS_TYPES - 1);
	xoffset = randomint(0, (PLATFORM_WIDTH-grass_width(type))*10.0f)/10.0f;
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
