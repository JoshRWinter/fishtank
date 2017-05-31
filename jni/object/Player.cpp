#include <math.h>
#include "../fishtank.h"

Player::Player(){
	colorid=0;
	x=0.0f;
	y=0.0f;
	xv=0.0f;
	yv=0.0f;
	w=PLAYER_WIDTH;
	h=PLAYER_HEIGHT;
	rot=0.0f;
	health=100;
	cue_fire=0.0f;
	frame=0;
	count=2;

	turret.w=TURRET_WIDTH;
	turret.h=TURRET_HEIGHT;
	turret.rot=M_PI;
	turret.count=1;
	turret.frame=0;

	beacon.y=FLOOR+10.0f;

	audio.bubbles=NULL;
	audio.engine=NULL;
}

void Player::process(State &state){
	int index=0;
	for(Player &player:state.player_list){

		// turn off empty players' sound effects
		if(player.colorid==0){
			if(player.audio.bubbles){
				stopsound(state.soundengine,player.audio.bubbles);
				player.audio.bubbles=NULL;
			}
			if(player.audio.engine){
				stopsound(state.soundengine,player.audio.engine);
				player.audio.engine=NULL;
			}
		}

		// handle bubble sound effects
		bool bubble_sound=false;
		if(&player==&state.player_list[state.match.my_index]){
			if(player.health>0&&(state.input.up_l.active||state.input.up_r.active))
				bubble_sound=true;
		}
		else if(player.health>0&&player.yv<0.0f&&inrange(state.player_list[state.match.my_index],player,SOUND_RANGE))
			bubble_sound=true;
		// play bubble sound
		if(bubble_sound){
			if(!player.audio.bubbles){
				player.audio.bubbles=playsound(state.soundengine,state.aassets.sound+SID_BUBBLES,true);
			}
		}
		else if(player.audio.bubbles){
			stopsound(state.soundengine,player.audio.bubbles);
			player.audio.bubbles=NULL;
		}

		// handle engine sound effects
		bool engine_sound=false;
		if(&player==&state.player_list[state.match.my_index]){
			if(player.health>0&&(state.input.left.active||state.input.right.active))
				engine_sound=true;
		}
		else if(player.health>0&&fabsf(player.xv)&&inrange(state.player_list[state.match.my_index],player,SOUND_RANGE))
			engine_sound=true;
		// play engine sound
		if(engine_sound){
			if(!player.audio.engine)
				player.audio.engine=playsound(state.soundengine,state.aassets.sound+SID_ENGINE,true);
		}
		else if(player.audio.engine){
			stopsound(state.soundengine,player.audio.engine);
			player.audio.engine=NULL;
			// play halting sound
			playsound(state.soundengine,state.aassets.sound+SID_ENGINE_HALT,false);
		}

		if(&player==&state.player_list[state.match.my_index]&&player.health<1&&state.match.dead_timer>0){
			--state.match.dead_timer;
			if(state.match.dead_timer<0)
				state.match.dead_timer=0;
		}

		// handle firing the cannon
		if(player.cue_fire>0.0f){
			if(player.health>0){
				// sound effect
				if(inrange(state.player_list[state.match.my_index],player,SOUND_RANGE))
					playsound(state.soundengine,state.aassets.sound+SID_CANNON,false);
				state.shell_list.push_back(new Shell(state,player));
			}
			else
				state.match.cycle_spectate(state.player_list); // cycle the spectated player
			player.cue_fire=0.0f;
		}

		// process the beacon
		// inactive if below FLOOR+3.0f
		if(player.beacon.y<FLOOR+3.0f){
			player.beacon.x+=player.beacon.xv;
			player.beacon.y+=player.beacon.yv;
			player.beacon.timer_frame-=state.speed;
			if(player.beacon.timer_frame<=0.0f){
				player.beacon.frame=!player.beacon.frame;
				player.beacon.timer_frame=BEACON_FRAME_TIMER;
			}
		}

		// spawn some bubble particles
		if(player.health>0&&!onein(3)){
			if(state.match.my_index==index){
				if(state.input.up_l.active||state.input.up_r.active)
					state.particle_bubble_list.push_back(new ParticleBubble(state,player));
			}
			else if(player.yv<0.0f)
				state.particle_bubble_list.push_back(new ParticleBubble(state,player));
		}

		player.x+=player.xv*state.speed;
		player.y+=player.yv*state.speed;

		player.turret.x=player.x+(PLAYER_WIDTH/2.0f)-(TURRET_WIDTH/2.0f);
		player.turret.y=player.y+(PLAYER_HEIGHT/2.0f)-(TURRET_HEIGHT*2.5f);

		state.final_firepower=0.0f;
		state.final_strikepower=0.0f;

		// tell the renderer the player's position
		if(&player==&state.player_list[state.match.my_index]){
			if(state.match.dead_timer==0){
				// spectating
				if(state.player_list[state.match.spectate_index].colorid==0||state.player_list[state.match.spectate_index].health<1)
					state.match.cycle_spectate(state.player_list);
				state.renderer.player_x=state.player_list[state.match.spectate_index].x;
				state.renderer.player_y=state.player_list[state.match.spectate_index].y;
			}
			else{
				state.renderer.player_x=player.x;
				state.renderer.player_y=player.y;
			}
		}

		++index;
	}
}

void Player::render(const Renderer &renderer,const std::vector<Player> &player_list){
	// render beacons
	bool bound=false;
	for(const Player &player:player_list){
		if(player.beacon.y>FLOOR+3.0f||player.colorid==0)
			continue;

		if(!bound){
			bound=true;
			glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_BEACON].object);
			glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		}

		renderer.draw(player.beacon);
	}

	// render the player's turret
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_TURRET].object);
	for(const Player &player:player_list){
		if(player.colorid==0)
			break;

		if(player.health<1)
			continue;

		// figure out the color;
		float r,g,b;
		State::fill_color(player.colorid,&r,&g,&b);
		glUniform4f(renderer.uniform.rgba,r,g,b,1.0f);

		renderer.draw(player.turret);
	}

	// render the player
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_TANK].object);
	for(const Player &player:player_list){
		if(player.colorid==0)
			break;
		if(player.health<1)
			continue;

		// figure out the color;
		float r,g,b;
		State::fill_color(player.colorid,&r,&g,&b);
		glUniform4f(renderer.uniform.rgba,r,g,b,1.0f);

		renderer.draw(player);
		if(player.health<100){
			glUniform4f(renderer.uniform.rgba,r,g,b,(100-player.health)/100.0f);
			// render the damage mask
			Player mask=player;
			mask.frame=1;
			renderer.draw(mask);
			glUniform4f(renderer.uniform.rgba,r,g,b,1.0f);
		}
	}
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
}

Beacon::Beacon(){
	w=BEACON_WIDTH;
	h=BEACON_HEIGHT;
	rot=0.0f;
	frame=0;
	count=2;
	x=0.0f;
	y=FLOOR+10.0f;
	timer_frame=BEACON_FRAME_TIMER;
}
