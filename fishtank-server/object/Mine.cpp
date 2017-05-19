#include "../fishtank-server.h"

uint32_t Mine::mine_status=0;

Mine::Mine(const std::vector<Platform> &platform_list,int platform_index){
	w=MINE_SIZE;
	h=MINE_SIZE;
	x=platform_list[platform_index].x+(PLATFORM_WIDTH/2.0f)-(MINE_SIZE/2.0f);
	y=platform_list[platform_index].y-MINE_SIZE-MINE_CHAIN_HEIGHT+0.2f;

	this->platform_index=platform_index;
	yv=0.0f;
	armed=true;
}

void Mine::create_all(Match &match){
	match.mine_list.clear();
	const int count=randomint(5,6); // how many mines

	for(int i=0;i<count;++i){
		// pick a platform
		bool picked=false;
		while(!picked){
			int platform_index=0;
			for(const Platform &platform:match.platform_list){
				if(!platform.horiz||platform.health<1)
					continue;

				// don't put two mines on one platform
				bool occupied=false;
				for(const Mine &mine:match.mine_list){
					if(platform_index==mine.platform_index){
						occupied=true;
						break;
					}
				}
				if(occupied){
					++platform_index;
					continue;
				}

				if(onein(20)){
					Mine mine(match.platform_list,platform_index);
					match.mine_list.push_back(mine);
					picked=true;
					break;
				}

				++platform_index;
			}
		}
	}
}

void Mine::process(Match &match){
	// populate Mine::mine_status
	Mine::mine_status=0;

	int i=0;
	// fill the status variable
	for(Mine &mine:match.mine_list){
		uint32_t status=mine.armed;

		Mine::mine_status|=status<<i;

		++i;

		// regular processing
		if(mine.armed){
			// update ypos
			mine.y+=mine.yv;

			// check to see if the platform the mine is attached to is destroyed
			// if so, the mine will be allowed to float upward
			if(match.platform_list[mine.platform_index].health<1){
				if(mine.yv==0.0f)
					mine.disturbed_by=match.platform_list[mine.platform_index].killed_by_id;

				mine.yv-=MINE_UPWARD_ACCEL;
				if(mine.yv<-MINE_UPWARD_TERMINAL_VEL)
					mine.yv=-MINE_UPWARD_TERMINAL_VEL;
			}

			// collides with platforms
			for(const Platform &platform:match.platform_list){
				if(platform.health<1)
					continue;

				if(mine.collide(platform)){
					mine.explode(match.platform_list,match.client_list);
				}
			}
		}
	}
}

void Mine::explode(std::vector<Platform> &platform_list,std::vector<Client*> &client_list){
	armed=false; // makes it inactive
	// destroy nearby platforms
	for(Platform &platform:platform_list){
		if(platform.health<1)
			continue;

		float d=distance(x+(MINE_SIZE/2.0f),platform.x+(platform.w/2.0f),y+(MINE_SIZE/2.0f),platform.y+(platform.h/2.0f));

		if(d<MINE_BLAST_RADIUS){
			platform.health=0;
			platform.killed_by_id=disturbed_by;
		}
	}

	// destroy nearby players
	for(Client *c:client_list){
		Client &client=*c;

		if(client.player.health<1)
			continue;

		float d=distance(x+(MINE_SIZE/2.0f),client.player.x+(PLAYER_WIDTH/2.0f),y+(MINE_SIZE/2.0f),client.player.y+(PLAYER_HEIGHT/2.0f));
		client.player.health-=Mine::dmg(d);
		if(client.player.health<1){
			client.kill_reason=KILLED_BY_MINE;
			client.killed_by_id=disturbed_by;
			client.player.health=0;
		}
	}
}

int Mine::dmg(float d){
	if(d<2.75f)
		return 100;
	if(d<3.4f)
		return 60;
	if(d<3.65f)
		return 50;
	if(d<3.85f)
		return 40;
	if(d<4.0)
		return 20;
	return 0;
}
