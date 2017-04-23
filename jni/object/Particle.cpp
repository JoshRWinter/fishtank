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
		renderer.draw(*p);
	}
}
