#include <math.h>
#include "../fishtank.h"

Platform::Platform(bool platform_active,bool horiz,float xpos,float ypos){
	if(horiz){
		w=PLATFORM_WIDTH;
		h=PLATFORM_HEIGHT;
		visual.rot=0.0f;
	}
	else{
		w=PLATFORM_HEIGHT;
		h=PLATFORM_WIDTH;
		visual.rot=M_PI/2.0f;
	}
	x=xpos-(w/2.0f);
	y=ypos-(h/2.0f);
	active=platform_active;

	visual.x=xpos-(PLATFORM_VIS_WIDTH/2.0f);
	visual.y=ypos-(PLATFORM_VIS_HEIGHT/2.0f);
	visual.w=PLATFORM_VIS_WIDTH;
	visual.h=PLATFORM_VIS_HEIGHT;
	visual.count=3;
	visual.frame=randomint(0,2);
}

void Platform::render(const Renderer &renderer,const std::vector<Platform> &platform_list){
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_PLATFORM].object);
	for(const Platform &platform:platform_list){
		if(!platform.active)
			continue;

		renderer.draw(platform.visual);
	}
}
