#include <string.h>
#include <math.h>
#include "../fishtank-server.h"

static float shell_dmg(float f){
	if(f<0.4)
		return 0.2f;
	if(f<0.5)
		return 0.45f;
	if(f<0.6)
		return 0.75f;
	if(f<0.7)
		return 0.85f;
	if(f<0.8)
		return 0.9f;
	if(f<0.9)
		return 0.97f;
	return 1.85f;
}

Player::Player(const area_bounds &bounds,const std::vector<Mine> &mine_list,int player_count){
	reset(bounds,mine_list);
	if(player_count>2)
		health=0;
}

void Player::reset(const area_bounds &bounds,const std::vector<Mine> &mine_list){
	health=100;
	w=PLAYER_WIDTH;
	h=PLAYER_HEIGHT;
	int tries=0;
	do{
		x=randomint(bounds.left*10.0f,bounds.right*10.0f)/10.0f;
		y=randomint((bounds.top-1.0f)*10.0f,bounds.bottom*10.0f)/10.0f;
		++tries;
	}while(near_mine(mine_list)&&tries<40);
	xv=0.0f;
	yv=0.0f;
	timer_fire=0;
	angle=M_PI/2.0f;
	avail_airstrike=true;
	beacon.reset();
}

bool Player::near_mine(const std::vector<Mine> &mine_list)const{
	for(const Mine &mine:mine_list){
		if(collide(mine,-4.0f))
			return true;
	}

	return false;
}

void Player::process(Match &match){
	for(Client *c:match.client_list){
		Client &client=*c;
		if(client.colorid==0)
			continue;

		// handle firing the cannon
		if(client.input.fire>0.0f&&client.player.health>0){
			client.player.timer_fire=PLAYER_TIMER_FIRE;
			Shell *s=new Shell(match,client,shell_dmg(client.input.fire));
			match.shell_list.push_back(s);
			// launch the player backwards a bit
			client.player.xv=-s->xv/5.0f;
		}
		if(client.player.timer_fire)
			--client.player.timer_fire;
		client.input.fire=0.0f;

		// handle firing a beacon (airstrike)
		if(client.player.avail_airstrike&&client.input.astrike>0.0f&&client.player.health>0){
			client.player.avail_airstrike=false;
			client.player.beacon.fire(client);
			client.input.astrike=0.0f;
			match.airstrike_list.push_back(new Airstrike(client));
		}
		else
			client.input.astrike=0.0f;

		// process the beacon
		// inactive if below FLOOR+3.0f
		if(client.player.beacon.y<FLOOR+3.0f){
			client.player.beacon.yv+=GRAVITY;

			float speed=sqrtf((client.player.beacon.xv*client.player.beacon.xv)+(client.player.beacon.yv*client.player.beacon.yv));

			client.player.beacon.x+=client.player.beacon.xv;
			client.player.beacon.y+=client.player.beacon.yv;
			client.player.beacon.rot+=client.player.beacon.rotv;

			// collides with platforms
			for(const Platform &platform:match.platform_list){
				if(platform.health<1)
					continue;

				int side;
				side=client.player.beacon.correct(platform);
				switch(side){
				case COLLIDE_TOP:
					if(speed>0.1f)
						client.player.beacon.rotv=client.player.beacon.xv*1.8f;
					else
						client.player.beacon.rotv*=0.79f;
					client.player.beacon.yv=-client.player.beacon.yv*0.3f;
					client.player.beacon.xv*=0.7f;
					break;
				case COLLIDE_LEFT:
				case COLLIDE_RIGHT:
					client.player.beacon.xv=-client.player.beacon.xv*0.4f;
					break;
				case COLLIDE_BOTTOM:
					client.player.beacon.yv=0.0f;
					break;
				}
			}

			// stop if below FLOOR
			if(client.player.beacon.y+BEACON_HEIGHT>FLOOR){
				if(speed>0.1f)
					client.player.beacon.rotv=client.player.beacon.xv*1.8f;
				else
					client.player.beacon.rotv*=0.79f;
				client.player.beacon.yv=-client.player.beacon.yv*0.3f;
				client.player.beacon.xv*=0.7f;
				client.player.beacon.y=FLOOR-BEACON_HEIGHT;
			}
		}

		// bring velocities down to zero
		const float ACCELERATE=0.0015f;
		if(!client.input.left&&!client.input.right)
			zerof(&client.player.xv,ACCELERATE);

		// handle velocities
		bool in_air=client.player.yv!=0.0f; // player is flying around
		float accel;
		float max;
		if(in_air){
			accel=ACCELERATE*1.6f;
			max=PLAYER_X_FAST_SPEED;
		}
		else{
			accel=ACCELERATE;
			max=PLAYER_X_SPEED;
		}
		if(client.input.left)
			targetf(&client.player.xv,accel,-max);
		if(client.input.right)
			targetf(&client.player.xv,accel,max);
		if(client.input.up)
			targetf(&client.player.yv,ACCELERATE,-PLAYER_Y_SPEED);

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
			if(!client.input.up)
				client.player.yv+=PLAYER_GRAVITY;

			// update pos based on velocities
			client.player.x+=client.player.xv;
			client.player.y+=client.player.yv;

			if(client.player.y+PLAYER_HEIGHT>FLOOR){
				client.player.y=FLOOR-PLAYER_HEIGHT;
				client.player.yv=0.0f;
			}
			if(client.player.x<LEFT){
				client.player.x=LEFT;
				client.player.xv=0.0f;
			}
			else if(client.player.x+PLAYER_WIDTH>RIGHT){
				client.player.x=RIGHT-PLAYER_WIDTH;
				client.player.xv=0.0f;
			}
		}
		else{
			client.player.xv=0.0f;
			client.player.yv=0.0f;
		}

		// check for player colliding with mines
		if(client.player.health>0){
			for(Mine &mine:match.mine_list){
				if(!mine.armed)
					continue;

				if(client.player.collide(mine,0.15f)){
					if(mine.disturbed_by==-1)
						mine.disturbed_by=client.id;
					mine.explode(match.platform_list,match.client_list);
				}
			}
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

		// check for player going to high
		if(client.player.health>0&&client.player.y<LOWER_CEILING){
			const float dmg=((-client.player.y)-(-LOWER_CEILING))/25.0f;
			client.player.health-=dmg;
			if(client.player.health<1){
				client.player.health=0;
				client.kill_reason=KILLED_BY_DECOMPRESSION;
				client.killed_by_id=client.id;
			}
		}

		// process dead clients
		if(client.kill_reason!=0){
			// find the client responsible for this
			Client *perp=NULL;
			for(Client *x:match.client_list){
				if(x->id==client.killed_by_id){
					perp=x;
					break;
				}
			}

			if(perp==NULL){
				// if the murderer disconnected, then perp could be NULL
				client.kill_reason=0;
				client.player.health=1; // reanimate the victim
			}
			else{ // most common
				std::string msg;
				// send a message describing who was killed and why
				// tell the victim who killed them
				to_client_tcp tctcp;
				memset(&tctcp,0,sizeof(tctcp));
				tctcp.type=TYPE_KILLER_INDEX;
				client.tcp.send(&tctcp.type,sizeof(tctcp.type));
				client.tcp.send(tctcp.msg,sizeof(tctcp.msg));
				client.tcp.send(tctcp.name,sizeof(tctcp.name));
				uint32_t killer_index=htonl(match.get_client_index(client.killed_by_id));
				client.tcp.send(&killer_index,sizeof(killer_index));

				// send a server message
				switch(client.kill_reason){
				case KILLED_BY_AIRSTRIKE:
					msg=perp->name+" bombed "+client.name+"!";
					break;
				case KILLED_BY_SHELL:
					msg=perp->name+" destroyed "+client.name+"!";
					break;
				case KILLED_BY_MINE:
					msg=perp->name+" mined "+client.name+"!";
					break;
				case KILLED_BY_DECOMPRESSION:
					msg=client.name+" died of\ndecompression sickness :(";
					break;
				}
				match.send_chat(msg,"server");

				// update the stats for the perp and the victim
				if(perp!=&client)
					++perp->stat.victories;
				++client.stat.deaths;
			}

			// prevent this code path from being triggered again
			client.kill_reason=0;
		}
	}
}



Beacon::Beacon(){
	reset();
}

void Beacon::reset(){
	w=BEACON_WIDTH;
	h=BEACON_HEIGHT;
	x=0.0f;
	y=FLOOR+10.0f;
	xv=0.0f;
	yv=0.0f;
}

void Beacon::fire(const Client &client){
	float turretx=client.player.x+(PLAYER_WIDTH/2.0f)-(TURRET_WIDTH/2.0f);
	float turrety=client.player.y+(PLAYER_HEIGHT/2.0f)-(TURRET_HEIGHT*2.5f);
	x=turretx+(TURRET_WIDTH/2.0f)-(BEACON_WIDTH/2.0f);
	y=turrety+(TURRET_HEIGHT/2.0f)-(BEACON_HEIGHT/2.0f);

	const float speed=client.input.astrike/2.4f;
	float xvel=-cosf(client.player.angle);
	float yvel=-sinf(client.player.angle);
	xv=xvel*speed;
	yv=yvel*speed;
	x+=xvel*0.6f;
	y+=yvel*0.6f;

	rot=client.player.angle;
	rotv=0.0f;
}
