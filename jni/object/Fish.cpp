#include "../fishtank.h"

DeadFish::DeadFish(float xpos,float ypos,int cid){
	w=FISH_WIDTH;
	h=FISH_HEIGHT;
	x=xpos-(FISH_WIDTH/2.0f);
	y=ypos-(FISH_HEIGHT/2.0f);
	rot=0.0f;
	texture=AID_DEADFISH;

	colorid=cid;
	increase=randomint(0,10);
	xv=0.0f;
	yv=0.0f;
	rotv=randomint(0,1)?-0.01f:0.01f;
	stuck=false;
}

void DeadFish::process(State &state){
	for(std::vector<DeadFish*>::iterator it=state.dead_fish_list.begin();it!=state.dead_fish_list.end();){
		DeadFish &fish=**it;

		// floats upwards
		fish.yv-=0.0006f*state.speed;
		if(fish.yv<-0.1f)
			fish.yv=-0.1f;

		// sine wave based on DeadFish::increase
		if(fish.stuck)
			fish.xv*=0.999f*state.speed;
		else
			fish.xv=(sinf(fish.increase)/30.0f)*state.speed;

		fish.increase+=0.09f;

		// update positions
		fish.x+=fish.xv*state.speed;
		fish.y+=fish.yv*state.speed;
		fish.rot+=fish.rotv*state.speed;

		// collide with level geometry
		for(const Platform &platform:state.platform_list){
			// skip dead platforms
			if(!platform.active)
				continue;

			int side=fish.correct(platform);
			switch(side){
			case 0: // ignore
				if(fish.yv<0.0f)
					fish.stuck=false;
				break;
			case COLLIDE_BOTTOM:
				fish.stuck=true;
				fish.xv=0.0f;
				fish.yv=0.0f;
				fish.rotv=0.0f;
				break;
			case COLLIDE_LEFT:
			case COLLIDE_RIGHT:
				fish.xv=0.0f;
				break;
			}
		}

		// delete it if it goes offscreen
		if(fish.y<state.renderer.view.top*6.0f){
			delete *it;
			it=state.dead_fish_list.erase(it);
			continue;
		}

		++it;
	}
}

void DeadFish::render(const Renderer &renderer,const std::vector<DeadFish*> &fish_list){
	for(const DeadFish *f:fish_list){
		float r,g,b;
		State::fill_color(f->colorid,&r,&g,&b);
		glUniform4f(renderer.uniform.rgba,r,g,b,1.0f);
		renderer.draw(*f,&renderer.atlas);
	}

	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
}
