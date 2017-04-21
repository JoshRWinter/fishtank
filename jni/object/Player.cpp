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
	count=1;

	turret.w=TURRET_WIDTH;
	turret.h=TURRET_HEIGHT;
	turret.rot=M_PI;
	turret.count=1;
	turret.frame=0;
}

void Player::process(State &state){
	for(Player &player:state.player_list){
		// handle firing the cannon
		if(player.cue_fire>0.0f){
			state.shell_list.push_back(new Shell(state,player));
			player.cue_fire=0.0f;
		}
		player.x+=player.xv*state.speed;
		player.y+=player.yv*state.speed;

		player.turret.x=player.x+(PLAYER_WIDTH/2.0f)-(TURRET_WIDTH/2.0f);
		player.turret.y=player.y+(PLAYER_HEIGHT/2.0f)-(TURRET_HEIGHT*2.5f);

		state.final_firepower=0.0f;

		// tell the renderer the player's position
		if(&player==&state.player_list[state.match.my_index]){
			state.renderer.player_x=player.x;
			state.renderer.player_y=player.y;
		}
	}
}

void Player::render(const Renderer &renderer,const std::vector<Player> &player_list){
	// render the player's turret
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_TANK].object);
	for(const Player &player:player_list){
		if(player.colorid==0)
			break;
		renderer.draw(player.turret);
	}

	// render the player
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_TANK].object);
	for(const Player &player:player_list){
		if(player.colorid==0)
			break;

		renderer.draw(player);
	}
}
