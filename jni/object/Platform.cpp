#include <math.h>
#include "../fishtank.h"

Platform::Platform(bool platform_active,bool horiz,float xpos,float ypos,unsigned s){
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
	texture=-1;

	visual.x=xpos-(PLATFORM_VIS_WIDTH/2.0f);
	visual.y=ypos-(PLATFORM_VIS_HEIGHT/2.0f);
	visual.w=PLATFORM_VIS_WIDTH;
	visual.h=PLATFORM_VIS_HEIGHT;
	visual.texture=AID_PLATFORM_1+randomint(0,2);

	health=100;
	timer_audio=-1.0f;
	seed=s;
}

void Platform::process(State &state){
	for(Platform &platform:state.platform_list){
		if(!platform.active){
			if(platform.timer_audio>=0.0f){
				platform.timer_audio-=state.speed;
				if(platform.timer_audio<=0.0f&&state.config.sounds)
					sl_play_stereo(state.soundengine,state.aassets.sound+SID_PLATFORM_DESTROY,platform.x+(platform.w/2.0f),platform.y+(platform.h/2.0f));
			}
		}
	}
}

void Platform::render(const Renderer &renderer,const std::vector<Platform> &platform_list){
	for(const Platform &platform:platform_list){
		if(!platform.active)
			continue;

		renderer.draw(platform.visual,&renderer.atlas);
		if(platform.health<100){
			// draw damage mask
			glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,(100-platform.health)/100.0f);
			Base dmg=platform.visual;
			dmg.texture=AID_PLATFORM_DMG;
			renderer.draw(dmg,&renderer.atlas);
			glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		}
	}
}
