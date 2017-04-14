#include "../fishtank.h"

Player::Player(){
	colorid=0;
	x=0.0f;
	y=0.0f;
	w=PLAYER_WIDTH;
	h=PLAYER_HEIGHT;
	rot=0.0f;
	health=100;
	cue_fire=false;
	frame=0;
	count=1;
}

void Player::render(const Renderer &renderer,const std::vector<Player> &player_list){
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_TANK].object);
	for(Player player:player_list){
		if(player.colorid==0)
			return;

		renderer.draw(player);
	}
}
