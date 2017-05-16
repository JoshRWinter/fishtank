#include "../fishtank.h"

ParticleShell::ParticleShell(const Shell &shell){
	w=0.0f;
	h=PARTICLE_SHELL_HEIGHT;
	rot=shell.visual.rot+(randomint(-40,40)*(M_PI/180.0f));
	x=shell.x+(shell.w/2.0f);
	y=shell.y+(SHELL_HEIGHT/2.0f)-(PARTICLE_SHELL_HEIGHT/2.0f);
	count=1;
	frame=0;

	float speed=randomint(16,23)/100.0f;
	xv=-cosf(rot)*speed;
	yv=-sinf(rot)*speed;
	ttl=randomint(PARTICLE_SHELL_TTL);
	colorid=shell.owner.colorid; // get color id from this shell's owner
}

void ParticleShell::spawn(State &state,const Shell &shell,int count){
	for(int i=0;i<count;++i){
		state.particle_shell_list.push_back(new ParticleShell(shell));
	}
}

void ParticleShell::process(State &state){
	for(std::vector<ParticleShell*>::iterator it=state.particle_shell_list.begin();it!=state.particle_shell_list.end();){
		ParticleShell &particle=**it;

		// affected by gravity
		particle.yv+=GRAVITY*state.speed;

		// update position
		particle.x+=particle.xv*state.speed;
		particle.y+=particle.yv*state.speed;
		float speed=sqrtf((particle.xv*particle.xv)+(particle.yv*particle.yv));
		particle.w=speed;

		// point the particle in the direction it's moving
		particle.rot=
		atan2f((particle.y+(PARTICLE_SHELL_HEIGHT/2.0f))-((particle.y+(PARTICLE_SHELL_HEIGHT/2.0f))+(particle.yv*state.speed)),
				(particle.x+(particle.w/2.0f))-((particle.x+(particle.w/2.0f))+(particle.xv*state.speed)));

		// delete the particle if ttl <= 0.0f
		particle.ttl-=state.speed;
		if(particle.ttl<=0.0f){
			delete *it;
			it=state.particle_shell_list.erase(it);
			continue;
		}

		++it;
	}
}

void ParticleShell::render(const Renderer &renderer,const std::vector<ParticleShell*> &particle_shell_list){
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_SHELL].object);

	for(const ParticleShell *p:particle_shell_list){
		// figure out the color;
		float r,g,b;
		State::fill_color(p->colorid,&r,&g,&b);
		glUniform4f(renderer.uniform.rgba,r,g,b,1.0f);

		renderer.draw(*p);
	}
}

ParticlePlatform::ParticlePlatform(const Shell &shell){
	w=PARTICLE_PLATFORM_SIZE;
	h=PARTICLE_PLATFORM_SIZE;
	rot=randomint(1,360)*(M_PI/180.0f);
	x=shell.x+(SHELL_WIDTH/2.0f)-(PARTICLE_PLATFORM_SIZE/2.0f);
	y=shell.y+(SHELL_HEIGHT/2.0f)-(PARTICLE_PLATFORM_SIZE/2.0f);
	count=2;
	frame=randomint(0,1);

	xv=-cosf(rot)*PARTICLE_PLATFORM_SPEED;
	yv=-sinf(rot)*PARTICLE_PLATFORM_SPEED;
	ttl=PARTICLE_PLATFORM_TTL;
}

ParticlePlatform::ParticlePlatform(float xpos,float ypos){
	w=PARTICLE_PLATFORM_SIZE;
	h=PARTICLE_PLATFORM_SIZE;
	rot=randomint(1,360)*(M_PI/180.0f);
	x=xpos-(PARTICLE_PLATFORM_SIZE/2.0f);
	y=ypos-(PARTICLE_PLATFORM_SIZE/2.0f);
	count=2;
	frame=randomint(0,1);

	xv=-cosf(rot)*PARTICLE_PLATFORM_SPEED/2.0f;
	yv=-sinf(rot)*PARTICLE_PLATFORM_SPEED/2.0f;
	ttl=PARTICLE_PLATFORM_TTL;
}

ParticlePlatform::ParticlePlatform(const Artillery &arty){
	w=PARTICLE_PLATFORM_SIZE;
	h=PARTICLE_PLATFORM_SIZE;
	rot=randomint(1,360)*(M_PI/180.0f);
	x=arty.x+(ARTILLERY_WIDTH/2.0f)-(PARTICLE_PLATFORM_SIZE/2.0f);
	y=arty.y+(ARTILLERY_HEIGHT/2.0f)-(PARTICLE_PLATFORM_SIZE/2.0f);
	count=2;
	frame=randomint(0,1);

	const float speed=randomint(10,17)/100.0f;
	xv=-cosf(rot)*speed;
	yv=-sinf(rot)*speed;
	ttl=PARTICLE_PLATFORM_TTL;
}

void ParticlePlatform::spawn(State &state,const Shell &shell,int count){
	for(int i=0;i<count;++i){
		state.particle_platform_list.push_back(new ParticlePlatform(shell));
	}
}

void ParticlePlatform::spawn_destroy_platform(State &state,const Platform &platform){
	for(int i=0;i<25;++i){
		const float SCALE=100.0f;
		float x=randomint(platform.x*SCALE,(platform.x+platform.w)*SCALE)/SCALE;
		float y=randomint(platform.y*SCALE,(platform.y+platform.h)*SCALE)/SCALE;

		state.particle_platform_list.push_back(new ParticlePlatform(x,y));
	}
}

void ParticlePlatform::spawn(State &state,const Artillery &arty){
	int count=randomint(22,29);
	for(int i=0;i<count;++i)
		state.particle_platform_list.push_back(new ParticlePlatform(arty));
}

void ParticlePlatform::process(State &state){
	for(std::vector<ParticlePlatform*>::iterator it=state.particle_platform_list.begin();it!=state.particle_platform_list.end();){
		ParticlePlatform &particle=**it;

		// affected by gravity
		particle.yv+=GRAVITY*state.speed;

		// update position
		particle.x+=particle.xv*state.speed;
		particle.y+=particle.yv*state.speed;
		particle.rot+=particle.xv*state.speed*6.0f;

		// check for particles colliding with platforms
		bool on_ground=false;
		for(const Platform &platform:state.platform_list){
			if(!platform.active)
				continue;

			int side=particle.correct(platform);
			switch(side){
			case COLLIDE_LEFT:
			case COLLIDE_RIGHT:
				particle.xv*=0.6f;
				particle.xv=-particle.xv;
				break;
			case COLLIDE_TOP:
				on_ground=true;
			case COLLIDE_BOTTOM:
				particle.yv*=0.4f;
				particle.yv=-particle.yv;
				break;
			}
		}

		// slow down horizontally
		if(on_ground)
			particle.xv*=0.95f;

		// delete when ttl <= 0.0f
		particle.ttl-=state.speed;
		if(particle.ttl<=0.0){
			// but shrink it first
			const float SHRINK=0.99f;
			float oldsize=particle.w;
			particle.w*=SHRINK;
			particle.h=particle.w;
			particle.x+=(oldsize-particle.w)/2.0f;
			particle.y+=(oldsize-particle.h)/2.0f;
			if(particle.w<=0.01f){
				// delete
				delete *it;
				it=state.particle_platform_list.erase(it);
				continue;
			}
		}

		++it;
	}
}

void ParticlePlatform::render(const Renderer &renderer,const std::vector<ParticlePlatform*> &particle_platform_list){
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_PARTICLE_PLATFORM].object);

	for(const ParticlePlatform *p:particle_platform_list)
		renderer.draw(*p);
}

ParticlePlayer::ParticlePlayer(float xpos,float ypos,bool large,int player_colorid){
	w=large?PARTICLE_PLAYER_LARGE_SIZE:PARTICLE_PLAYER_SIZE;
	h=w;
	x=xpos-(w/2.0f);
	y=ypos-(h/2.0f);
	rot=randomint(1,360)*(M_PI/180.0f);
	count=1;
	frame=0;

	float speed;
	if(large)
		speed=randomint(19,24)/100.0f;
	else
		speed=randomint(10,14)/100.0f;
	xv=-cosf(rot)*speed;
	yv=-sinf(rot)*speed;
	rotv=xv;
	ttl=500.0f;
	active=true;
	colorid=player_colorid;
}

void ParticlePlayer::spawn(State &state,const Shell &shell,int colorid){
	const int count=randomint(2,5);
	for(int i=0;i<count;++i){
		state.particle_player_list.push_back(new ParticlePlayer(shell.x+(SHELL_WIDTH/2.0f)+shell.xv,shell.y+(SHELL_HEIGHT/2.0f)+shell.yv,false,colorid));
	}
}

void ParticlePlayer::spawn(State &state,const Player &player){
	const int count=randomint(12,15);
	for(int i=0;i<count;++i){
		state.particle_player_list.push_back(new ParticlePlayer(player.x+(PLAYER_WIDTH/2.0f),player.y+(PLAYER_HEIGHT/2.0f),true,player.colorid));
	}
	// spawn one dead fish
	state.dead_fish_list.push_back(new DeadFish(player.x+(PLAYER_WIDTH/2.0f),player.y+(PLAYER_HEIGHT/2.0f),player.colorid));
}

void ParticlePlayer::process(State &state){
	for(std::vector<ParticlePlayer*>::iterator it=state.particle_player_list.begin();it!=state.particle_player_list.end();){
		ParticlePlayer &particle=**it;

		// affected by gravity
		if(particle.active){
			particle.yv+=GRAVITY*state.speed;
		}

		// update positions
		particle.x+=particle.xv*state.speed;
		particle.y+=particle.yv*state.speed;
		particle.rot+=particle.rotv;

		// check for particles colliding with ground
		particle.active=true;
		if(particle.y+particle.h>FLOOR){
			particle.xv=0.0f;
			particle.yv=0.0f;
			particle.rotv=0.0f;
			particle.active=false;
		}
		else{
			// check for particles colliding with platforms
			for(const Platform &platform:state.platform_list){
				if(!platform.active)
					continue;

				if(particle.collide(platform)){
					particle.xv=0.0f;
					particle.yv=0.0f;
					particle.rotv=0.0f;
					particle.active=false;
					break;
				}
			}
		}

		// delete when ttl <= 0.0f
		particle.ttl-=state.speed;
		if(particle.ttl<=0.0f){
			// shrink it first
			const float SHRINK=0.99f;
			float oldsize=particle.w;
			particle.w*=SHRINK;
			particle.h=particle.w;
			particle.x+=(oldsize-particle.w)/2.0f;
			particle.y+=(oldsize-particle.h)/2.0f;
		}

		++it;
	}
}

void ParticlePlayer::render(const Renderer &renderer,const std::vector<ParticlePlayer*> &particle_player_list){
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_PARTICLE_PLAYER].object);
	for(const ParticlePlayer *p:particle_player_list){
		// figure out the color;
		float r,g,b;
		State::fill_color(p->colorid,&r,&g,&b);
		glUniform4f(renderer.uniform.rgba,r,g,b,1.0f);

		renderer.draw(*p);
	}
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
}

// intense: larger bubbles
ParticleBubble::ParticleBubble(const State &state,const Player &player){
	w=PARTICLE_BUBBLE_SIZE;
	h=PARTICLE_BUBBLE_SIZE;

	// pick a side of the player to spawn this bubble on
	const float inward=0.3f;
	int left_side=randomint(0,1);
	if(left_side)
		x=player.x+inward-(w/2.0f);
	else
		x=(player.x+PLAYER_WIDTH)-inward-(w/2.0f);
	y=player.y+PLAYER_HEIGHT-0.3f;

	rot=randomint(1,360)*(M_PI/180.0);
	xv=randomint(-3,3)/100.0f;
	yv=(randomint(12,14)/100.0f)*state.speed;
	ttl=20.0f;
	count=1;
	frame=0;
}

ParticleBubble::ParticleBubble(const Artillery &arty){
	w=PARTICLE_BUBBLE_SIZE;
	h=PARTICLE_BUBBLE_SIZE;
	x=arty.x+(ARTILLERY_WIDTH/2.0f)-(PARTICLE_BUBBLE_SIZE/2.0f);
	y=arty.y+(ARTILLERY_HEIGHT/2.0f)-(PARTICLE_BUBBLE_SIZE/2.0f);
	rot=randomint(1,360)*(M_PI/180.0);
	ttl=30.0f;
	count=1;
	frame=0;

	xv=randomint(-1,1)/90.0f;
	yv=randomint(-1,1)/90.0f;
}

void ParticleBubble::process(State &state){
	for(std::vector<ParticleBubble*>::iterator it=state.particle_bubble_list.begin();it!=state.particle_bubble_list.end();){
		ParticleBubble &particle=**it;
		bool kill=false;

		// slowly decrease yv
		particle.yv*=0.995;

		// update positions
		particle.x+=particle.xv*state.speed;
		particle.y+=particle.yv*state.speed;

		// collides with level geometry
		for(const Platform &platform:state.platform_list){
			// skip inactive platforms
			if(!platform.active)
				continue;

			int side=particle.correct(platform);
			switch(side){
			case 0: // ignore
				break;
			case COLLIDE_LEFT:
			case COLLIDE_RIGHT:
				particle.xv=-particle.xv;
				break;
			case COLLIDE_TOP:
				if(onein(4))
					kill=true;
				particle.yv=-particle.yv*0.1f;
				particle.xv*=-1.6f;
				break;
			}
		}

		// if ttl<=0.0f delete
		particle.ttl-=state.speed;
		if(particle.ttl<=0.0f){
			// shrink it
			float oldsize=particle.w;
			particle.w-=0.01f;
			particle.h=particle.w;
			particle.x+=(oldsize-particle.w)/2.0f;
			particle.y+=(oldsize-particle.h)/2.0f;

			if(particle.w<=0.0f)
				kill=true;
		}

		if(kill){
			delete *it;
			it=state.particle_bubble_list.erase(it);
			continue;
		}

		++it;
	}
}

void ParticleBubble::render(const Renderer &renderer,const std::vector<ParticleBubble*> &particle_list){
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_PARTICLE_BUBBLE].object);

	for(const ParticleBubble *p:particle_list)
		renderer.draw(*p);
}
