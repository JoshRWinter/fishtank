#include <math.h>
#include "../fishtank.h"

Shell::Shell(const State &state,const Player &player):owner(player){
	w=SHELL_WIDTH;
	h=SHELL_HEIGHT;
	x=player.turret.x+(TURRET_WIDTH/2.0f)-(SHELL_WIDTH/2.0f);
	y=player.turret.y+(TURRET_HEIGHT/2.0f)-(SHELL_HEIGHT/2.0f);
	rot=0.0f;
	count=1;
	frame=0;

	const float speed=player.cue_fire/2.4f;
	float xvel=-cosf(player.turret.rot);
	float yvel=-sinf(player.turret.rot);
	xv=xvel*speed;
	yv=yvel*speed;
	x+=xvel*0.6f;
	y+=yvel*0.6f;

	visual.w=SHELL_VIS_WIDTH;
	visual.h=0.0f;
	visual.rot=player.turret.rot;
	visual.frame=0;
	visual.count=1;
}

void Shell::process(State &state){
	for(std::vector<Shell*>::iterator it=state.shell_list.begin();it!=state.shell_list.end();){
		Shell &shell=**it;

		// update shell velocities
		shell.yv+=GRAVITY*state.speed;

		// update shell position
		shell.x+=shell.xv*state.speed;
		shell.y+=shell.yv*state.speed;
		float speed=sqrtf((shell.xv*shell.xv)+(shell.yv*shell.yv));
		shell.visual.w=(speed<0.2f?0.2f:speed)*3.0f;
		shell.visual.x=shell.x+(SHELL_WIDTH/2.0f)-(shell.visual.w/2.0f);
		shell.visual.y=shell.y+(SHELL_HEIGHT/2.0f)-(shell.visual.h/2.0f);
		// grow the shell's height
		targetf(&shell.visual.h,speed/2.5f,SHELL_VIS_HEIGHT);
		// turn the shell to the direction it's travelling
		shell.visual.rot=atan2f(
			(shell.visual.y+(SHELL_VIS_HEIGHT/2.0f))-((shell.visual.y+(SHELL_VIS_HEIGHT/2.0f))+shell.yv),
			(shell.visual.x+(shell.visual.w/2.0f))-((shell.visual.x+(shell.visual.w/2.0f))+shell.xv));

		// check for shells colliding with player
		bool stop=false;
		for(Player &player:state.player_list){
			if(&player==&shell.owner||player.colorid==0||player.health<1)
				continue;

			if(player.collide(shell)){
				// sound effect
				if(state.config.sounds)
					playsound(state.soundengine,state.aassets.sound+SID_SHELL_PLAYER_IMPACT,State::attenuation(state.player_list[state.match.get_current_index()].dist(shell)),false);
				// vibrate
				if(&player==&state.player_list[state.match.get_current_index()])
					if(state.config.vibrate)
						vibratedevice(&state.jni,30);
				// generate some particles
				ParticleShell::spawn(state,shell,randomint(6,9));
				// more particles
				ParticlePlayer::spawn(state,shell,player.colorid);

				delete *it;
				it=state.shell_list.erase(it);
				stop=true;
				break;
			}

		}
		if(stop)
			continue;

		// check for shells colliding with platform
		for(Platform &platform:state.platform_list){
			if(!platform.active)
				continue;

			if(shell.collide(platform)){
				// sound effect
				if(state.config.sounds)
					playsound(state.soundengine,state.aassets.sound+SID_PLATFORM_IMPACT,State::attenuation(state.player_list[state.match.get_current_index()].dist(shell)),false);
				// generate some particles
				ParticlePlatform::spawn(state,shell,randomint(3,5));
				// estimate dmg
				platform.health-=25;
				delete *it;
				it=state.shell_list.erase(it);
				stop=true;
				break;
			}
		}
		if(stop)
			continue;

		// check for shells colliding with mines
		for(const Mine &mine:state.mine_list){
			if(!mine.armed)
				continue;

			if(shell.collide(mine)){
				// particles
				ParticleShell::spawn(state,shell,randomint(3,4));
				delete *it;
				it=state.shell_list.erase(it);
				stop=true;
				break;
			}
		}
		if(stop)
			continue;

		// check for shell going below screen
		if(shell.y>FLOOR-0.4f){
			// sound effect
			if(state.config.sounds)
				playsound(state.soundengine,state.aassets.sound+SID_PLATFORM_IMPACT,State::attenuation(state.player_list[state.match.get_current_index()].dist(shell)),false);
			// particles
			ParticlePlatform::spawn(state,shell,randomint(3,4));
			delete *it;
			it=state.shell_list.erase(it);
			continue;
		}

		++it;
	}
}

void Shell::render(const Renderer &renderer,const std::vector<Shell*> &shell_list){
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_SHELL].object);
	for(const Shell *shell:shell_list){
		// figure out the color;
		float r,g,b;
		State::fill_color(shell->owner.colorid,&r,&g,&b);
		glUniform4f(renderer.uniform.rgba,r,g,b,1.0f);

		renderer.draw(shell->visual);
	}
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
}
