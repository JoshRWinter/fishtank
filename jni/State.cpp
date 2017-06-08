#include "fishtank.h"

bool State::core(){
	if(show_menu){
		if(!menu.main.exec(*this))
			return false;
		show_menu=false;
		play_music();
	}

	// get network updates
	match.recv_data(*this);
	// send network updates
	match.send_data(*this);
	// see if an error happened with the above operations
	if(!match.connected()){
		show_menu=true;
		if(!menu.message.exec(*this,"You were disconnected because\nthe server doesn't like you anymore.","Disconnected"))
			return false;
	}

	// process music
	if(config.music)
		process_music();

	// process players
	Player::process(*this);

	// process shells
	Shell::process(*this);

	// process platforms
	Platform::process(*this);

	// process artillery
	Artillery::process(*this);

	// process mines
	Mine::process(*this);

	// process particle shells
	ParticleShell::process(*this);

	// process particle platforms
	ParticlePlatform::process(*this);

	// process particle players
	ParticlePlayer::process(*this);

	// process particle bubbles
	ParticleBubble::process(*this);

	// process dead fish
	DeadFish::process(*this);

	// process server messages
	if(announcement.size()>0){
		if(announcement.size()==1)
			announcement[0].timer-=speed;
		else if(announcement.size()>1)
			announcement[0].timer-=speed*3.0f;

		if(announcement[0].timer<=0.0f)
			announcement.erase(announcement.begin());
	}

	if(!pause_menu){
		// check for upper left chat press
		if(pointer[0].active&&pointer[0].x>renderer.view.left&&pointer[0].x<renderer.view.left+4.0f&&
			pointer[0].y>renderer.view.top&&pointer[0].y<renderer.view.top+2.0f){
			pause_menu=true;
			if(!menu.chat.exec(*this))
				return false;
			pause_menu=false;
		}

		// chatpane timer
		if(timer_chatpane>0.0f){
			timer_chatpane-=speed;
			if(timer_chatpane<0.0f)
				timer_chatpane=0.0f;
		}

		const float UI_TOLERANCE=-0.30f;
		if(match.get_current_index()==match.my_index&&match.dead_timer!=0){
			// process ui buttons
			input.left.process(pointer,UI_TOLERANCE);
			input.right.process(pointer,UI_TOLERANCE);
			input.up_r.process(pointer,UI_TOLERANCE);
			input.up_l.process(pointer,UI_TOLERANCE);
			input.aim_left.process(pointer,UI_TOLERANCE);
			input.aim_right.process(pointer,UI_TOLERANCE);
			if(input.strike.process(pointer,UI_TOLERANCE))
				strike_mode=!strike_mode;
					if(input.fire.process(pointer,UI_TOLERANCE)){
				const float minimum=0.35f;
				if(firepower<minimum)
					firepower=minimum;
				if(strike_mode){
					strike_mode=false;
					final_strikepower=firepower;
				}
				else
					final_firepower=firepower;
				firepower=0.0f;
			}
			else if(input.fire.active){
				firepower+=FIREPOWER_INCREMENT*speed;
				if(firepower>1.0f)
					firepower=1.0f;
			}
			else
				firepower=0.0f;
		}
		if(match.get_current_index()!=match.my_index&&match.dead_timer==0){
			if(input.cycle.process(pointer,UI_TOLERANCE))
				match.cycle_spectate(player_list); // cycle the spectated player
		}
	}

	// pause menu
	if(back){
		back=false;
		pause_menu=true;
		if(!menu.pause.exec(*this))
			return false;
		pause_menu=false;
	}

	return true;
}

void State::render()const{
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	// draw backdrop
	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[match.backdrop_index].object);
	renderer.draw(backdrop);
	// draw the upper gradient
	if(renderer.player_y-4.5f<LOWER_CEILING){
		glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_UPPER_GRADIENT].object);
		Base gradient={renderer.player_x-8.0f+(PLAYER_WIDTH/2.0f),UPPER_CEILING,16.0f,18.0f,0.0f,0,1};
		renderer.draw(gradient);
	}

	// draw dead fish
	if(dead_fish_list.size()!=0)
		DeadFish::render(renderer,dead_fish_list);

	// draw particle bubbles
	if(particle_bubble_list.size()!=0.0f)
		ParticleBubble::render(renderer,particle_bubble_list);

	// draw shells
	if(shell_list.size()!=0)
		Shell::render(renderer,shell_list);

	// draw mines
	if(mine_list.size()!=0)
		Mine::render(renderer,mine_list);

	// draw players
	Player::render(renderer,player_list);

	// draw particles players
	if(particle_player_list.size()!=0)
		ParticlePlayer::render(renderer,particle_player_list);

	// draw particle platforms
	if(particle_platform_list.size()!=0)
		ParticlePlatform::render(renderer,particle_platform_list);

	// draw platforms
	Platform::render(renderer,platform_list);

	// draw artillery
	if(arty_list.size()!=0)
		Artillery::render(renderer,arty_list);

	// draw particle shells
	if(particle_shell_list.size()!=0)
		ParticleShell::render(renderer,particle_shell_list);

	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	// draw the ground
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_GROUND].object);
	renderer.draw(ground);

	if(match.get_current_index()==match.my_index){
		// firepower indicator
		if(firepower>0.0f){
			glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_FIREPOWER_INDICATOR].object);
			float s=FIRE_BUTTON_SIZE*firepower;
			Base fpi={input.fire.x+(FIRE_BUTTON_SIZE/2.0f)-(s/2.0f),input.fire.y+(FIRE_BUTTON_SIZE/2.0f)-(s/2.0f),s,s,0.0f,0,1};
			renderer.uidraw(fpi);
		}
		// draw ui buttons
		glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_UIBUTTON].object);
		input.left.render(renderer);
		input.right.render(renderer);
		input.up_r.render(renderer);
		input.up_l.render(renderer);
		input.aim_left.render(renderer);
		input.aim_right.render(renderer);
		glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_FBUTTON].object);
		input.fire.render(renderer);
		glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON_AS].object);
		// artillery strike button
		if(strike_mode)
			glUniform4f(renderer.uniform.rgba,0.9f,0.15f,0.21f,0.13f);
		else if(!input.strike.active)
			glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,0.13f);
		else
			glUniform4f(renderer.uniform.rgba,0.7f,0.7f,0.7f,0.13f);
		renderer.uidraw(input.strike);
		// ui button text
		glBindTexture(GL_TEXTURE_2D,renderer.font.button_small->atlas);
		glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,0.3f);
		input.left.render_text(renderer);
		input.right.render_text(renderer);
		input.up_r.render_text(renderer);
		input.up_l.render_text(renderer);
		input.fire.render_text(renderer);
		input.aim_left.render_text(renderer);
		input.aim_right.render_text(renderer);
	}
	else{
		// "cycle" spectate button
		glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_FBUTTON].object);
		input.cycle.render(renderer);
		glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,0.3f);
		glBindTexture(GL_TEXTURE_2D,renderer.font.button_small->atlas);
		input.cycle.render_text(renderer);
	}

	// draw server and chat messages
	if(announcement.size()>0){
		glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
		glUniform4f(renderer.uniform.rgba,0.701f,0.71f,0.3f,1.0f);
		drawtextcentered(renderer.font.main,0.0f,-3.15f,announcement[0].msg.c_str());
	}
	if(chat.size()>0&&timer_chatpane>0.0f&&!pause_menu){
		glUniform4f(renderer.uniform.rgba,TEXT_COLOR,1.0f);
		glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);

		// draw the last 3 messages
		int index=chat.size()-3;
		if(index<0)
			index=0;
		float line=renderer.view.top+0.1f;
		while(index<chat.size()){
			std::string entry=chat[index].from+": "+chat[index].msg;
			drawtext(renderer.font.main,renderer.view.left+0.1f,line,entry.c_str());
			line+=0.6f;
			++index;
		}
	}

	// draw "spectating ..." string
	if(match.dead_timer==0){
		glUniform4f(renderer.uniform.rgba,0.701f,0.71f,0.3f,1.0f);
		glBindTexture(GL_TEXTURE_2D,renderer.font.button->atlas);
		std::string spectating;
		spectating="Shadowing: "+match.spectate_name;
		drawtextcentered(renderer.font.button,0.0f,1.25f,spectating.c_str());
	}

	// fps
#ifdef SHOW_FPS
	{
		static char fps_string[36];
		static int last_time,fps;
		int current_time=time(NULL);
		if(current_time!=last_time){
			last_time=current_time;
			sprintf(fps_string,"[fishtank] fps: %d",fps);
			fps=0;
		}
		else
			++fps;

		glUniform4f(renderer.uniform.rgba,0.0f,0.0f,0.0f,1.0f);
		glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
		drawtext(renderer.font.main,renderer.view.right-5.0f,renderer.view.top+0.1f,fps_string);
	}
#endif // SHOW_FPS
}

bool State::process()const{
	// poll for events
	int events;
	android_poll_source *source;
	while(ALooper_pollAll(running?0:-1,NULL,&events,(void**)&source)>=0){
		if(source)
			source->process(app,source);
		if(app->destroyRequested)
			return false;
	}
	return true;
}

State::State(){
	if(!read_config()){
		name="basil fawlty";
		connect_to="";
		colorid=COLOR_RED;
		config.music=true;
		config.sounds=true;
		config.vibrate=true;
	}

	speed=1.0f;
	running=false;
	back=false;
	pause_menu=false;
	show_menu=true;
	memset(pointer,0,sizeof(crosshair)*2);
	firepower=0.0f;
	final_firepower=0.0f;
	final_strikepower=0.0f;

	// background
	backdrop.x=-13.5f;
	backdrop.y=-14.6f;
	backdrop.w=55.0f;
	backdrop.h=19.0f;
	backdrop.rot=0.0f;
	backdrop.count=1;
	backdrop.frame=0;

	// ground
	ground.x=-26.0f;
	ground.y=FLOOR-0.2f;
	ground.h=4.4f;
	ground.w=75.0f;
	ground.rot=0.0f;
	ground.frame=0;
	ground.count=1;

	// ui buttons
	const float DPAD_SIZE=1.2f;
	input.left.init(-7.1f,2.45f,DPAD_SIZE,"L");
	input.right.init(-4.4f,2.45f,DPAD_SIZE,"R");
	input.up_r.init(4.55f,0.8f,DPAD_SIZE,"U");
	input.up_l.init(-5.75f,0.8f,DPAD_SIZE,"U");
	input.fire.init(4.225f,2.3f,FIRE_BUTTON_SIZE,"FIRE");
	input.cycle.init(-FIRE_BUTTON_SIZE/2.0f,2.3f,FIRE_BUTTON_SIZE,"NEXT");
	input.aim_left.init(2.7f,2.5f,DPAD_SIZE,"L");
	input.aim_right.init(6.35f,2.5f,DPAD_SIZE,"R");
	input.strike.init(-DPAD_SIZE/2.0f,3.05f,DPAD_SIZE,"");
}

void State::reset(){
	timer_chatpane=0.0f;
	strike_mode=false;

	// clear shells
	for(Shell *shell:shell_list)
		delete shell;
	shell_list.clear();
	// platforms are cleared in Match::get_level_config
	// mines are cleared in Match::get_level_config
	// clear particle shells
	for(ParticleShell *p:particle_shell_list)
		delete p;
	particle_shell_list.clear();
	// clear artillery
	for(Artillery *a:arty_list)
		delete a;
	arty_list.clear();
	// clear particle platforms
	for(ParticlePlatform *p:particle_platform_list)
		delete p;
	particle_platform_list.clear();
	// clear particle players
	for(ParticlePlayer *p:particle_player_list)
		delete p;
	particle_player_list.clear();
	// clear particle bubbles
	for(ParticleBubble *p:particle_bubble_list)
		delete p;
	particle_bubble_list.clear();
	// clear dead fish
	for(DeadFish *f:dead_fish_list)
		delete f;
	dead_fish_list.clear();

	// clear messages
	chat.clear();
	announcement.clear();

	// reset players
	player_list.clear();
	Player dummy;
	for(int i=0;i<MAX_PLAYERS;++i){
		player_list.push_back(dummy);
	}
}

void State::init(android_app &app){
	loadapack(&aassets,app.activity->assetManager,"aassets");
	soundengine=initOpenSL(sound_config_fn);
	play_music();
}

void State::play_music(){
	sl_stop_all(soundengine);
	if(config.music){
		if(show_menu){
			// play menu theme
			sl_play_loop(soundengine,aassets.sound+SID_MENU_THEME);
		}
		else{
			// play the gameplay theme
			music.track=2;
			music.id=sl_play(soundengine,aassets.sound+music.track);
			music.check_timer=0;
		}
	}
	else{
		// play silence
		sl_play_loop(soundengine,aassets.sound+SID_SILENCE);
	}
}

void State::process_music(){
	if(music.check_timer==0){
		music.check_timer=200;
		// check if the music is still playing
		bool playing=sl_check(soundengine,music.id)==1;
		if(!playing){
			// find next track
			++music.track;
			if(music.track>3)
				music.track=1;

			// play more music
			music.id=sl_play(soundengine,aassets.sound+music.track);
		}
	}
	else
		--music.check_timer;
}

void State::term(){
	termOpenSL(soundengine);
	destroyapack(&aassets);
}

bool State::read_config(){
	FILE *file=fopen(DATAPATH"/00","rb");

	if(!file)
		return false;
	char connect_to_tmp[MSG_LIMIT+1];
	char name_tmp[MSG_LIMIT+1];
	fread(connect_to_tmp,1,MSG_LIMIT+1,file);
	fread(name_tmp,1,MSG_LIMIT+1,file);
	fread(&colorid,sizeof(int),1,file);
	name=name_tmp;
	connect_to=connect_to_tmp;
	char music,sounds,vibrate;
	fread(&music,sizeof(char),1,file);
	fread(&sounds,sizeof(char),1,file);
	fread(&vibrate,sizeof(char),1,file);
	config.music=music!=0;
	config.sounds=sounds!=0;
	config.vibrate=vibrate!=0;

	fclose(file);
	return true;
}

void State::write_config(){
	FILE *file=fopen(DATAPATH"/00","wb");
	if(!file){
		logcat("could not open " DATAPATH "/00 for writing");
		return;
	}

	char connect_to_tmp[MSG_LIMIT+1];
	char name_tmp[MSG_LIMIT+1];
	strncpy(connect_to_tmp,connect_to.c_str(),MSG_LIMIT+1);
	strncpy(name_tmp,name.c_str(),MSG_LIMIT+1);
	fwrite(connect_to_tmp,1,MSG_LIMIT+1,file);
	fwrite(name_tmp,1,MSG_LIMIT+1,file);
	fwrite(&colorid,sizeof(int),1,file);
	char music=config.music;
	char sounds=config.sounds;
	char vibrate=config.vibrate;
	fwrite(&music,sizeof(char),1,file);
	fwrite(&sounds,sizeof(char),1,file);
	fwrite(&vibrate,sizeof(char),1,file);

	fclose(file);
}

void State::fill_color(int id,float *r,float *g,float *b){
	switch(id){
	default:
	case COLOR_RED:
		*r=RGB_RED_R;*g=RGB_RED_G;*b=RGB_RED_B;
		break;
	case COLOR_BLUE:
		*r=RGB_BLUE_R;*g=RGB_BLUE_G;*b=RGB_BLUE_B;
		break;
	case COLOR_CYAN:
		*r=RGB_CYAN_R;*g=RGB_CYAN_G;*b=RGB_CYAN_B;
		break;
	case COLOR_GREEN:
		*r=RGB_GREEN_R;*g=RGB_GREEN_G;*b=RGB_GREEN_B;
		break;
	case COLOR_PURPLE:
		*r=RGB_PURPLE_R;*g=RGB_PURPLE_G;*b=RGB_PURPLE_B;
		break;
	}
}
