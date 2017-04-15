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
	cue_fire=false;
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
		player.x+=player.xv;
		player.y+=player.yv;

		player.turret.x=player.x+(PLAYER_WIDTH/2.0f)-(TURRET_WIDTH/2.0f);
		player.turret.y=player.y+(PLAYER_HEIGHT/2.0f)-(TURRET_HEIGHT*2.5f);
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
