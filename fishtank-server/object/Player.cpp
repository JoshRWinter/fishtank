#include <math.h>
#include "../fishtank-server.h"

Player::Player(){
	reset();
}

void Player::reset(){
	health=100;
	x=0.0f;
	y=0.0f;
	xv=0.0f;
	yv=0.0f;
	angle=M_PI/2.0f;
	w=PLAYER_WIDTH;
	h=PLAYER_HEIGHT;
}

void Player::process(Match &match){
	for(Client *c:match.client_list){
		Client &client=*c;
		if(client.colorid==0)
			continue;

		// handle firing the cannon
		if(client.input.fire>0.0f&&client.player.health>0){
			Shell *s=new Shell(match,client);
			match.shell_list.push_back(s);
			client.input.fire=0.0f;
			// launch the player backwards a bit
			client.player.xv=-s->xv/2.0f;
		}

		// bring velocities down to zero
		const float RETARD=0.01f;
		zerof(&client.player.xv,RETARD);

		// handle velocities
		if(client.input.left)
			client.player.xv=-PLAYER_X_SPEED;
		if(client.input.right)
			client.player.xv=PLAYER_X_SPEED;
		if(client.input.up)
			client.player.yv=-PLAYER_Y_SPEED;
		/*
		if(client.input.down)
			client.player.yv=PLAYER_Y_SPEED;
		*/

		// handle turret rotation
		if(client.input.aim_left){
			client.player.angle-=PLAYER_ANGLE_INCREMENT;
			if(client.player.angle<0.3f)
				client.player.angle=0.3f;
		}
		else if(client.input.aim_right){
			client.player.angle+=PLAYER_ANGLE_INCREMENT;
			if(client.player.angle>M_PI-0.3f)
				client.player.angle=M_PI-0.3f;
		}

		if(client.player.health>0){
			// affected by gravity
			client.player.yv+=GRAVITY;

			// update pos based on velocities
			client.player.x+=client.player.xv;
			client.player.y+=client.player.yv;

			if(client.player.y+PLAYER_HEIGHT>FLOOR){
				client.player.y=FLOOR-PLAYER_HEIGHT;
				client.player.yv=0.0f;
			}
		}
		else{
			client.player.xv=0.0f;
			client.player.yv=0.0f;
		}

		// check for player colliding with platform
		for(const Platform &platform:match.platform_list){
			if(platform.health<1)
				continue;

			int side=client.player.correct(platform);
			switch(side){
			case 0: // no collision
				break;
			case COLLIDE_RIGHT:
			case COLLIDE_LEFT:
				client.player.xv=0.0f;
				break;
			case COLLIDE_TOP:
			case COLLIDE_BOTTOM:
				client.player.yv=0.0f;
				break;
			}
		}
	}
}
