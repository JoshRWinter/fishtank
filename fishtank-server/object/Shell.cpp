#include "../fishtank-server.h"

Shell::Shell(const Match &match,Client &client,float firepower):owner(client),power(firepower){
	w=SHELL_WIDTH;
	h=SHELL_HEIGHT;
	float turretx=client.player.x+(PLAYER_WIDTH/2.0f)-(TURRET_WIDTH/2.0f);
	float turrety=client.player.y+(PLAYER_HEIGHT/2.0f)-(TURRET_HEIGHT*2.5f);
	x=turretx+(TURRET_WIDTH/2.0f)-(SHELL_WIDTH/2.0f);
	y=turrety+(TURRET_HEIGHT/2.0f)-(SHELL_HEIGHT/2.0f);

	const float speed=client.input.fire/2.4f;
	float xvel=-cosf(client.player.angle);
	float yvel=-sinf(client.player.angle);
	xv=xvel*speed;
	yv=yvel*speed;
	x+=xvel*0.6f;
	y+=yvel*0.6f;
}

void Shell::process(Match &match){
	for(std::vector<Shell*>::iterator it=match.shell_list.begin();it!=match.shell_list.end();){
		Shell &shell=**it;

		shell.yv+=GRAVITY;
		shell.x+=shell.xv;
		shell.y+=shell.yv;

		// check for shells colliding with players
		bool stop=false;
		for(Client *c:match.client_list){
			Client &client=*c;

			if(&client==&shell.owner||client.colorid==0||client.player.health<1)
				continue;

			if(client.player.collide(shell)){
				// push the player back and subtract health
				client.player.xv+=shell.xv/2.5f;
				client.player.health-=randomint(SHELL_DMG)*shell.power;

				// see if it killed the player
				if(client.player.health<1){
					// take credit for the kill
					client.killed_by_id=shell.owner.id;
					client.kill_reason=KILLED_BY_SHELL;
				}

				delete *it;
				it=match.shell_list.erase(it);
				stop=true;
				break;
			}
		}
		if(stop)
			continue;

		// check for shells colliding with platforms
		for(Platform &platform:match.platform_list){
			if(platform.health<1)
				continue;

			if(shell.collide(platform)){
				platform.health-=25;
				if(platform.health<0)
					platform.health=0;
				delete *it;
				it=match.shell_list.erase(it);
				stop=true;
				break;
			}
		}
		if(stop)
			continue;

		// check for shells going below the screen
		if(shell.y>VIEW_BOTTOM){
			delete *it;
			it=match.shell_list.erase(it);
			continue;
		}

		++it;
	}
}
