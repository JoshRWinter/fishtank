#include "fishtank.h"

bool State::core(){
	if(show_menu){
		show_menu=false;
		if(!menu.main.exec(*this))
			return false;
	}

	// get network updates
	match.recv_data(*this);
	// send network updates
	match.send_data(*this);
	// see if an error happened with the above operations
	if(!match.connected()){
		logcat("error, disconnected");
		show_menu=true;
	}

	// process players
	Player::process(*this);

	// process shells
	Shell::process(*this);

	// process artillery
	Artillery::process(*this);

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
			announcement[0].timer-=speed*2.0f;

		if(announcement[0].timer<=0.0f)
			announcement.erase(announcement.begin());
	}

	if(!pause_menu){
		// check for upper left chat press
		if(pointer[0].active&&pointer[0].x>renderer.view.left&&pointer[0].x<renderer.view.left+4.0f&&
			pointer[0].y>renderer.view.top&&pointer[0].y<renderer.view.top+2.0f){
			if(!menu.chat.exec(*this))
				return false;
		}

		// chatpane timer
		if(timer_chatpane>0.0f){
			timer_chatpane-=speed;
			if(timer_chatpane<0.0f)
				timer_chatpane=0.0f;
		}

		// process ui buttons
		const float UI_TOLERANCE=-0.25f;
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
				final_strokepower=firepower;
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

	// pause menu
	if(back){
		back=false;
		pause_menu=true;
		menu.pause.exec(*this);
		pause_menu=false;
	}

	return true;
}

void State::render()const{
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	// draw backdrop
	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_BACKDROP].object);
	renderer.draw(backdrop);


	// draw dead fish
	if(dead_fish_list.size()!=0)
		DeadFish::render(renderer,dead_fish_list);

	// draw particle bubbles
	if(particle_bubble_list.size()!=0.0f)
		ParticleBubble::render(renderer,particle_bubble_list);

	// draw shells
	if(shell_list.size()!=0)
		Shell::render(renderer,shell_list);

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
		glUniform4f(renderer.uniform.rgba,0.9f,0.15f,0.21f,0.18f);
	else if(!input.strike.active)
		glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,0.18f);
	else
		glUniform4f(renderer.uniform.rgba,0.7f,0.7f,0.7f,0.18f);
	renderer.uidraw(input.strike);
	// ui button text
	glBindTexture(GL_TEXTURE_2D,renderer.font.button_small->atlas);
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,0.4f);
	input.left.render_text(renderer);
	input.right.render_text(renderer);
	input.up_r.render_text(renderer);
	input.up_l.render_text(renderer);
	input.fire.render_text(renderer);
	input.aim_left.render_text(renderer);
	input.aim_right.render_text(renderer);

	// draw server and chat messages
	if(announcement.size()>0){
		glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
		glUniform4f(renderer.uniform.rgba,0.701f,0.71f,0.3f,1.0f);
		drawtextcentered(renderer.font.main,0.0f,-3.15f,announcement[0].msg.c_str());
	}
	if(chat.size()>0&&timer_chatpane>0.0f){
		glUniform4f(renderer.uniform.rgba,BUTTON_TEXT_COLOR,1.0f);
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
	}

	speed=1.0f;
	running=false;
	back=false;
	pause_menu=false;
	show_menu=true;
	memset(pointer,0,sizeof(crosshair)*2);
	firepower=0.0f;
	final_firepower=0.0f;
	final_strokepower=0.0f;

	// players
	Player dummy;
	for(int i=0;i<MAX_PLAYERS;++i){
		player_list.push_back(dummy);
	}

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
	const float DPAD_SIZE=1.1f;
	input.left.init(-7.0f,2.25f,DPAD_SIZE,"L");
	input.right.init(-4.5f,2.25f,DPAD_SIZE,"R");
	input.up_r.init(5.0f,0.7f,DPAD_SIZE,"U");
	input.up_l.init(-5.75f,1.0f,DPAD_SIZE,"U");
	input.fire.init(4.625f,2.0f,FIRE_BUTTON_SIZE,"FIRE");
	input.aim_left.init(3.3f,2.5f,DPAD_SIZE,"L");
	input.aim_right.init(6.75f,2.5f,DPAD_SIZE,"R");
	input.strike.init(-DPAD_SIZE/2.0f,3.25f,DPAD_SIZE,"");
}

void State::reset(){
	timer_chatpane=0.0f;
	strike_mode=false;

	// clear shells
	for(Shell *shell:shell_list)
		delete shell;
	shell_list.clear();
	// platforms are cleared in Match::get_level_config
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
