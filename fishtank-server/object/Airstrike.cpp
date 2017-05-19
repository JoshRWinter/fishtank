#include "../fishtank-server.h"

Airstrike::Airstrike(const Client &c):client(c){
	count=ARTILLERY_COUNT;
	timer=ARTILLERY_INITIAL_TIMER;
}

Airstrike::~Airstrike(){
	// clear the arty_list
	for(Artillery *arty:arty_list)
		delete arty;
}

void Airstrike::process(Match &match){
	for(std::vector<Airstrike*>::iterator it=match.airstrike_list.begin();it!=match.airstrike_list.end();){
		Airstrike &strike=**it;

		// run the timer down, spawn an artillery
		--strike.timer;
		if(strike.timer<1&&strike.count>0){
			strike.arty_list.push_back(new Artillery(strike.client.player.beacon,strike.client.id));
			strike.timer=randomint(ARTILLERY_TIMER);
			--strike.count;
		}

		// process the artilleries
		for(std::vector<Artillery*>::iterator arty_it=strike.arty_list.begin();arty_it!=strike.arty_list.end();){
			Artillery &arty=**arty_it;

			// move it downward
			arty.y+=ARTILLERY_DOWNWARD_VELOCITY;

			// update x pos
			arty.x+=arty.xv;

			// collides with level geometry
			bool stop=false;
			for(Platform &platform:match.platform_list){
				if(platform.health<1)
					continue;

				// destroy the platform
				// maybe delete the projectile
				if(arty.collide(platform)){
					platform.health=0;
					platform.killed_by_id=arty.client_id;
					arty.explode(match.client_list);
					if(platform.seed%2==0){
						// delete the arty
						delete *arty_it;
						arty_it=strike.arty_list.erase(arty_it);
						stop=true;
						break;
					}
				}
			}
			if(stop)
				continue;

			// delete if at FLOOR
			if(arty.y+ARTILLERY_SIZE>FLOOR){
				arty.explode(match.client_list);
				delete *arty_it;
				arty_it=strike.arty_list.erase(arty_it);
				continue;
			}

			++arty_it;
		}

		// delete the airstrike
		if(strike.count<1&&strike.arty_list.size()==0){
			delete *it;
			match.airstrike_list.erase(it);
			continue;
		}

		++it;
	}
}

Artillery::Artillery(const Beacon &beacon,int id){
	x=beacon.x+(BEACON_WIDTH/2.0f);
	y=-20.0f;
	w=ARTILLERY_SIZE;
	h=ARTILLERY_SIZE;
	xv=randomint(-80,80)/1000.0f;
	processed=false;
	client_id=id;
}

void Artillery::explode(std::vector<Client*> &client_list){
	// blow up nearby players
	for(Client *c:client_list){
		Client &client=*c;

		if(c->player.health<1)
			continue;

		float d=distance(x+(ARTILLERY_SIZE/2.0f),client.player.x+(PLAYER_WIDTH/2.0f),y+(ARTILLERY_SIZE/2.0f),client.player.y+(PLAYER_HEIGHT/2.0f));
		if(d<ARTILLERY_SPLASH_RADIUS){
			client.player.health-=Artillery::dmg(d);
			if(client.player.health<1){
				client.killed_by_id=client_id;
				client.kill_reason=KILLED_BY_AIRSTRIKE;
				client.player.health=0;
			}
		}
	}
}

int Artillery::dmg(float d){
	if(d<1.5f)
		return 100;
	else if(d<2.0f)
		return 80;
	else if(d<3.0f)
		return 60;
	else if(d<4.0f)
		return 35;
	else if(d<5.0f)
		return 10;
	else
		return 0;
}
