#include "../fishtank-server.h"

Player::Player(){
	health=100;
	x=0.0f;
	y=0.0f;
	xv=0.0f;
	yv=0.0f;
}

void Player::process(Match &match){
	for(Client *c:match.client_list){
		Client &client=*c;
		if(client.colorid==0)
			continue;

		// handle velocities
		if(client.input.left)
			client.player.xv=-PLAYER_X_SPEED;
		if(client.input.right)
			client.player.xv=PLAYER_X_SPEED;
		if(!client.input.left&&!client.input.right)
			client.player.xv=0.0f;
		if(client.input.up)
			client.player.yv=-PLAYER_Y_SPEED;
		/*
		if(client.input.down)
			client.player.yv=PLAYER_Y_SPEED;
		*/

		// update pos based on velocities
		client.player.x+=client.player.xv;
		client.player.y+=client.player.yv;

		client.player.yv+=GRAVITY;
		if(client.player.y+PLAYER_HEIGHT>FLOOR){
			client.player.y=FLOOR-PLAYER_HEIGHT;
			client.player.yv=0.0f;
		}
	}
}
