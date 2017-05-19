#include "../fishtank.h"

Mine::Mine(const std::vector<Platform> &platform_list,int platform_index,bool armed){
	chain.w=MINE_CHAIN_WIDTH;
	chain.h=MINE_CHAIN_HEIGHT;
	chain.x=platform_list[platform_index].x+(PLATFORM_WIDTH/2.0f)-(MINE_CHAIN_WIDTH/2.0f);
	chain.y=platform_list[platform_index].y-MINE_CHAIN_HEIGHT;
	chain.rot=0.0f;
	chain.frame=0;
	chain.count=1;

	w=MINE_SIZE;
	h=MINE_SIZE;
	x=chain.x+(MINE_CHAIN_WIDTH/2.0f)-(MINE_SIZE/2.0f);
	y=chain.y-MINE_SIZE+0.2f;
	rot=randomint(1,360)*(M_PI/180.0f);
	frame=0;
	count=1;

	this->platform_index=platform_index;
	yv=0.0f;
	this->armed=armed;
}

void Mine::process(State &state){
	for(Mine &mine:state.mine_list){
		if(!mine.armed)
			continue;

		// check if the platform the mine is attatched to is destroyed
		if(!state.platform_list[mine.platform_index].active){
			// if so, the mine will be allowed to float upward
			mine.yv-=MINE_UPWARD_ACCEL;
			if(mine.yv<-MINE_UPWARD_TERMINAL_VEL)
				mine.yv=-MINE_UPWARD_TERMINAL_VEL;
		}

		// update ypos
		mine.y+=mine.yv*state.speed;

		// if mine is moving upward, make the chain appear to snap
		if(mine.yv<0.0f){
			if(mine.chain.h>0.0f){
				mine.chain.h-=0.5f;
				mine.chain.y+=0.25f;
				if(mine.chain.h<0.0f)
					mine.chain.h=0.0f;
			}
		}

		// collides with platforms
		for(const Platform &platform:state.platform_list){
			if(!platform.active)
				continue;

			if(mine.collide(platform))
				mine.y=platform.y+PLATFORM_HEIGHT;
		}
	}
}

void Mine::render(const Renderer &renderer,const std::vector<Mine> &mine_list){
	// draw the mine chain
	bool bound=false;
	for(const Mine &mine:mine_list){
		if(!mine.armed)
			continue;

		if(!bound){
			glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_MINE_CHAIN].object);
			bound=true;
		}

		renderer.draw(mine.chain);
	}

	// draw the mine
	bound=false;
	for(const Mine &mine:mine_list){
		if(!mine.armed)
			continue;

		if(!bound){
			glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_MINE].object);
			bound=true;
		}

		renderer.draw(mine);
	}

}
