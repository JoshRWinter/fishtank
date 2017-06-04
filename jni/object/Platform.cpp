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

	visual.x=xpos-(PLATFORM_VIS_WIDTH/2.0f);
	visual.y=ypos-(PLATFORM_VIS_HEIGHT/2.0f);
	visual.w=PLATFORM_VIS_WIDTH;
	visual.h=PLATFORM_VIS_HEIGHT;
	visual.count=4;
	visual.frame=randomint(0,2);

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
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_PLATFORM].object);
	for(const Platform &platform:platform_list){
		if(!platform.active)
			continue;

		renderer.draw(platform.visual);
		if(platform.health<100){
			glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,(100-platform.health)/100.0f);
			Base dmg=platform.visual;
			dmg.frame=3;
			renderer.draw(dmg);
			glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		}
	}
}
