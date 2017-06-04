#include "../fishtank.h"

Artillery::Artillery(float xpos,float xvel){
	w=ARTILLERY_SIZE;
	h=ARTILLERY_SIZE;
	x=xpos;
	y=ARTILLERY_Y;
	rot=0.0f;
	frame=0;
	count=1;

	xv=xvel;
	visual.w=ARTILLERY_WIDTH;
	visual.h=ARTILLERY_HEIGHT;
	// calculate rot from current direction
	visual.w=ARTILLERY_WIDTH;
	visual.h=ARTILLERY_HEIGHT;
	visual.x=x+(ARTILLERY_SIZE/2.0f)-(ARTILLERY_WIDTH/2.0f);
	visual.y=y+(ARTILLERY_SIZE/2.0f)-(ARTILLERY_HEIGHT/2.0f);
	visual.rot=atan2f((visual.y+(ARTILLERY_HEIGHT/2.0f))-((visual.y+ARTILLERY_DOWNWARD_VELOCITY)+(ARTILLERY_HEIGHT/2.0f)),
		(visual.x+(ARTILLERY_WIDTH/2.0f))-((visual.x+xv)+(ARTILLERY_WIDTH/2.0f)));
	visual.frame=0;
	visual.count=1;
}

void Artillery::process(State &state){
	for(std::vector<Artillery*>::iterator it=state.arty_list.begin();it!=state.arty_list.end();){
		Artillery &arty=**it;

		// gravity
		arty.y+=ARTILLERY_DOWNWARD_VELOCITY*state.speed;

		arty.x+=arty.xv*state.speed;

		// level geometry
		bool stop=false;
		for(const Platform &p:state.platform_list){
			if(!p.active)
				continue;

			if(arty.collide(p)){
				// vibrate
				if(inrange(state.player_list[state.match.get_current_index()],arty,6.0f))
					if(state.config.vibrate)
						vibratedevice(&state.jni,60);
				// just a visual effect, don't destroy the platform
				ParticlePlatform::spawn(state,arty);
				if(p.seed%2==0){
					delete *it;
					it=state.arty_list.erase(it);
					stop=true;
					break;
				}
			}
		}
		if(stop)
			continue;

		// delete if below FLOOR
		if(arty.y+ARTILLERY_SIZE>FLOOR){
			// vibrate and sound effects
			if(state.config.sounds)
				sl_play_stereo(state.soundengine,state.aassets.sound+SID_PLATFORM_DESTROY,arty.x+(ARTILLERY_SIZE/2.0f),arty.y+(ARTILLERY_SIZE/2.0f));
			if(inrange(state.player_list[state.match.get_current_index()],arty,6.75f)){
				if(state.config.vibrate)
					vibratedevice(&state.jni,40);
			}
			// particles
			ParticlePlatform::spawn(state,arty);
			delete *it;
			it=state.arty_list.erase(it);
			continue;
		}

		// synchronize with the visual
		arty.visual.x=arty.x+(ARTILLERY_SIZE/2.0f)-(ARTILLERY_WIDTH/2.0f);
		arty.visual.y=arty.y+(ARTILLERY_SIZE/2.0f)-(ARTILLERY_HEIGHT/2.0f);

		// spawn some bubblez
		int count=randomint(1,2);
		for(int i=0;i<count;++i)
			state.particle_bubble_list.push_back(new ParticleBubble(arty));

		++it;
	}
}

void Artillery::render(const Renderer &renderer,const std::vector<Artillery*> &arty_list){
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_ARTILLERY].object);
	for(const Artillery *a:arty_list)
		renderer.draw(a->visual);
}
