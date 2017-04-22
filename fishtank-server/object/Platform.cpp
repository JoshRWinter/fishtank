#include "../fishtank-server.h"

uint32_t Platform::platform_status=0;

Platform::Platform(bool horizontal,float xpos,float ypos){
	horiz=horizontal;
	if(horiz){
		w=PLATFORM_WIDTH;
		h=PLATFORM_HEIGHT;
	}
	else{
		w=PLATFORM_HEIGHT;
		h=PLATFORM_WIDTH;
	}
	x=xpos;//-(w/2.0f);
	y=ypos;//-(h/2.0f);
	rot=0.0f;
	health=100;
}

void Platform::create_all(Match &match){
	match.platform_list.clear();
	const float START_X=-5.0f;
	float x=START_X,y=0.5f;

	// create some horizontal lines
	for(int j=0;j<3;++j){
		for(int i=0;i<7;++i){
			if(onein(5)){
				x+=PLATFORM_WIDTH;
				continue;
			}

			Platform p(true,x,y);
			x+=PLATFORM_WIDTH;

			match.platform_list.push_back(p);
		}
		y-=PLATFORM_WIDTH;
		x=START_X;
	}

	// create some vertical lines
	for(Platform &horiz:match.platform_list){
		if(horiz.health<1)
			break;
		else if(onein(4))
			continue;
		else if(match.platform_list.size()==PLATFORM_COUNT)
			break;

		Platform p(false,horiz.x,horiz.y-PLATFORM_WIDTH);

		match.platform_list.push_back(p);
	}

	// fill up the rest
	int unused=0;
	while(match.platform_list.size()<PLATFORM_COUNT){
		++unused;
		Platform p(true,0.0f,0.0f);
		p.health=0;

		match.platform_list.push_back(p);
	}

	// just to check
	if(match.platform_list.size()!=PLATFORM_COUNT){
		std::cout<<"\033[31;1mERROR: platform_list.size() != PLATFORM_COUNT\033[0m"<<std::endl;
		exit(1);
	}

	std::cout<<"constructed new playing area ("<<unused<<" unused)"<<std::endl;
}

void Platform::process(const std::vector<Platform> &platform_list){
	Platform::platform_status=0;
	// pack all platform health status 1=alive 0=dead into uint32_t Platform::platform_status
	uint32_t i=0;
	for(const Platform &platform:platform_list){
		uint32_t status=platform_list[i].health>0;

		Platform::platform_status|=status<<i;

		++i;
	}
}
