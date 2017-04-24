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
	count=1;
	frame=0;

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
	count=1;
	frame=0;

	xv=-cosf(rot)*PARTICLE_PLATFORM_SPEED/2.0f;
	yv=-sinf(rot)*PARTICLE_PLATFORM_SPEED/2.0f;
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
