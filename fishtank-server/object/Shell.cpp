#include <math.h>
#include "../fishtank-server.h"

Shell::Shell(const Match &match,const Client &client):owner(client){
	w=SHELL_WIDTH;
	h=SHELL_HEIGHT;
	float turretx=client.player.x+(PLAYER_WIDTH/2.0f)-(TURRET_WIDTH/2.0f);
	float turrety=client.player.y+(PLAYER_HEIGHT/2.0f)-(TURRET_HEIGHT*2.5f);
	x=turretx+(TURRET_WIDTH/2.0f)-(SHELL_WIDTH/2.0f);
	y=turrety+(TURRET_HEIGHT/2.0f)-(SHELL_HEIGHT/2.0f);
	rot=0.0f;

	const float speed=client.input.fire/2.4f;
	xv=-cosf(client.player.angle)*speed;
	yv=-sinf(client.player.angle)*speed;
	x+=xv*2.0f;
	y+=yv*2.0f;
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

			if(&client==&shell.owner||client.colorid==0)
				continue;

			if(client.player.collide(shell)){
				client.player.xv=shell.xv;
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
