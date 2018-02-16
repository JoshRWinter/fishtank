#include "../fishtank-server.h"

uint32_t Platform::platform_status[2];

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
	x=xpos;
	y=ypos;
	health=100;

	killed_by_id=0;
	seed=rand()%100;
}

void Platform::create_all(Match &match){
	match.platform_list.clear();
	const float START_X=-12.0f;
	float x=START_X,y=-0.35f;

	// create some horizontal lines
	for(int j=0;j<4;++j){
		for(int i=0;i<13;++i){
			if(onein(5)){
				x+=PLATFORM_WIDTH;
				continue;
			}

			Platform p(true,x,y);
			x+=PLATFORM_WIDTH;

			match.platform_list.push_back(p);
		}
		y-=PLATFORM_WIDTH+PLATFORM_HEIGHT;
		x=START_X;
	}

	// create some vertical lines
	const int horiz_count = match.platform_list.size();
	for(int i = 0; i < horiz_count; ++i){
		if(onein(2))
			continue;
		else if(match.platform_list.size()==PLATFORM_COUNT)
			break;

		Platform p(false,match.platform_list[i].x-(PLATFORM_HEIGHT/2.0f),match.platform_list[i].y+PLATFORM_HEIGHT);

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

	// fill in Match::bounds
	match.bounds.left=match.bounds.right=match.bounds.bottom=match.bounds.top=0.0f;
	for(const Platform &p:match.platform_list){
		if(p.x<match.bounds.left)
			match.bounds.left=p.x;
		if(p.x>match.bounds.right)
			match.bounds.right=p.x;
		if(p.y<match.bounds.top)
			match.bounds.top=y;
		if(p.y>match.bounds.bottom)
			match.bounds.bottom=p.y;
	}
	match.bounds.left-=2.0f;
	match.bounds.right+=2.0f;
	match.bounds.bottom+=3.0f;
	match.bounds.top-=1.0f;

	// just to check
	if(match.platform_list.size()!=PLATFORM_COUNT){
		std::cout<<"\033[31;1mERROR: platform_list.size() != PLATFORM_COUNT\033[0m"<<std::endl;
		exit(1);
	}

	std::cout<<"constructed new playing area ("<<unused<<" unused)"<<std::endl;
}

// fill in platform_status
void Platform::update(const std::vector<Platform> &platform_list){
	Platform::platform_status[0]=0;
	Platform::platform_status[1]=0;
	// pack all platform health status 1=alive 0=dead into uint32_t Platform::platform_status
	for(unsigned i=0;i<32&&i<platform_list.size();++i){
		uint32_t status=platform_list[i].health>0;

		Platform::platform_status[0]|=status<<i;
	}
	for(unsigned i=0;i<32&&i<platform_list.size();++i){
		uint32_t status=platform_list[i+32].health>0;

		Platform::platform_status[1]|=status<<i;
	}
}

void Platform::process(const std::vector<Platform> &platform_list){
	Platform::update(platform_list);
}
